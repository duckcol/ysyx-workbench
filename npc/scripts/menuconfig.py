#!/usr/bin/env python3
"""
menuconfig.py — Lightweight interactive configuration for NPC.

Reads options from config_items.yaml, provides a curses TUI,
and generates conf.h + vsrc_conf.h.v.

Usage:  python3 scripts/menuconfig.py
        make menuconfig

Dependencies: Python 3.12+ stdlib + pyyaml
"""

import curses
import json
import os
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional

import yaml

# ── Paths ──────────────────────────────────────────────────
SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
CONF_H = PROJECT_ROOT / "csrc" / "include" / "conf.h"
CONF_H_BAK = PROJECT_ROOT / "csrc" / "include" / "conf.h.bak"
VSRC_CONF_H_V = PROJECT_ROOT / "vsrc" / "vsrc_conf.h.v"
VSRC_CONF_H_V_BAK = PROJECT_ROOT / "vsrc" / "vsrc_conf.h.v.bak"
CONFIG_JSON = PROJECT_ROOT / ".config.json"
ITEMS_YAML = SCRIPT_DIR / "config_items.yaml"


# ── Data Model ─────────────────────────────────────────────


@dataclass
class ConfigItem:
    """One configurable option, loaded from config_items.yaml."""

    name: str
    type: str = "bool"
    default: Any = True
    desc: str = ""
    category: str = ""
    depends: str = ""
    hidden: bool = False
    choice_group: str = ""
    target: str = "c"
    fmt: str = ""
    vmin: int = 0
    vmax: int = 0


def load_items(path: Path) -> List[ConfigItem]:
    """Parse config_items.yaml → list of ConfigItem."""
    with open(path) as f:
        data = yaml.safe_load(f)
    items = []
    for raw in data["items"]:
        # convert hex strings in YAML (e.g. "0x8000000") to int
        d = dict(raw)
        for key in ("default", "vmin", "vmax"):
            if key in d and isinstance(d[key], str) and d[key].startswith("0x"):
                d[key] = int(d[key], 16)
        d.setdefault("type", "bool")
        d.setdefault("desc", "")
        d.setdefault("category", "")
        d.setdefault("depends", "")
        d.setdefault("hidden", False)
        d.setdefault("choice_group", "")
        d.setdefault("target", "c")
        d.setdefault("fmt", "")
        d.setdefault("vmin", 0)
        d.setdefault("vmax", 0)
        items.append(ConfigItem(**{k: d[k] for k in [
            "name", "type", "default", "desc", "category",
            "depends", "hidden", "choice_group", "target",
            "fmt", "vmin", "vmax"
        ]}))
    return items


# ── State Management ───────────────────────────────────────


def load_state(items: List[ConfigItem]) -> Dict[str, Any]:
    """Load persisted state, falling back to item defaults."""
    if CONFIG_JSON.exists():
        with open(CONFIG_JSON) as f:
            saved = json.load(f)
        state = {}
        for item in items:
            state[item.name] = saved.get(item.name, item.default)
        return state
    return {item.name: item.default for item in items}


def save_state(state: Dict[str, Any]):
    """Persist current state."""
    with open(CONFIG_JSON, "w") as f:
        json.dump(state, f, indent=2,
                  default=lambda x: hex(x) if isinstance(x, int) else x)


def resolve_effective(
    items: List[ConfigItem], raw: Dict[str, Any]
) -> Dict[str, Any]:
    """Resolve dependency chains into effective (enforced) state."""
    item_map = {i.name: i for i in items}
    eff = raw.copy()

    changed = True
    while changed:
        changed = False
        for item in items:
            name = item.name

            # hidden items always mirror their parent
            if item.hidden and item.depends:
                pv = bool(eff.get(item.depends, False))
                if bool(eff[name]) != pv:
                    eff[name] = pv
                    changed = True
                continue

            # children forced off when parent is off
            if item.depends:
                parent = item.depends
                if parent in item_map and not bool(eff.get(parent, False)):
                    if bool(eff[name]):
                        eff[name] = False
                        changed = True

    # choice groups: exactly one selected
    groups: Dict[str, List[ConfigItem]] = {}
    for item in items:
        if item.choice_group:
            groups.setdefault(item.choice_group, []).append(item)
    for gname, members in groups.items():
        true_members = [m for m in members if bool(eff[m.name])]
        if len(true_members) == 0:
            eff[members[0].name] = True
        elif len(true_members) > 1:
            for m in true_members[1:]:
                eff[m.name] = False

    return eff


# ── Backup ─────────────────────────────────────────────────


def backup_once():
    """Copy old conf files to .bak (first run only)."""
    for src, dst in [(CONF_H, CONF_H_BAK), (VSRC_CONF_H_V, VSRC_CONF_H_V_BAK)]:
        if src.exists() and not dst.exists():
            shutil.copy2(src, dst)


# ── Header Generation ──────────────────────────────────────


def generate_conf_h(eff: Dict[str, Any], items: List[ConfigItem]):
    """Write conf.h (flat, no #ifdef nesting)."""
    mb = eff.get("CONFIG_MBASE", 0x80000000)
    ms = eff.get("CONFIG_MSIZE", 0x8000000)

    lines = [
        "// Auto-generated by scripts/menuconfig.py",
        "// DO NOT EDIT — run 'make menuconfig' to change settings.",
        "",
        "// ── Fixed ─────────────────────────────────────────────",
        "#define RESET_VECTOR CONFIG_MBASE",
        "",
        "#define CONFIG_RT_CHECK 1",
        "#define LLVM_VERSION_MAJOR 18",
        "",
        "#define CONFIG_ISA_riscv 1",
        "#define CONFIG_RVE 2",
        "",
    ]

    c_visible = [i for i in items if i.target in ("c", "both") and not i.hidden]
    c_hidden = [i for i in items if i.target in ("c", "both") and i.hidden]

    seen_cats = []
    for item in c_visible:
        if item.category not in seen_cats:
            seen_cats.append(item.category)
            lines.append(f"// ── {item.category} ──")
        val = eff.get(item.name, item.default)
        _emit_c(lines, item, val)
    lines.append("")

    if c_hidden:
        lines.append("// ── Implied (auto-managed) ──")
        for item in c_hidden:
            val = eff.get(item.name, item.default)
            _emit_c(lines, item, val)
        lines.append("")

    with open(CONF_H, "w") as f:
        f.write("\n".join(lines) + "\n")


def generate_vsrc_conf_h_v(eff: Dict[str, Any], items: List[ConfigItem]):
    """Write vsrc_conf.h.v."""
    mb = eff.get("CONFIG_MBASE", 0x80000000)

    lines = [
        "// Auto-generated by scripts/menuconfig.py",
        "// DO NOT EDIT — run 'make menuconfig' to change settings.",
        "",
        "`ifndef _VSRC_CONF_H_",
        "`define _VSRC_CONF_H_",
        "`endif",
        "",
        f"`define NPC_MEM_BASE 32'h{mb:08x}",
        "`define NPC_IR_RST_VAL 32'h000000013",
        "`define NPC_MSTATUS_RST_VAL 32'h1800",
        "",
        "// data width settings",
        "`define NPC_ADDR_LEN 32",
        "`define NPC_INST_LEN 32",
        "`define NPC_REG_ADDR_LEN 5",
        "`define NPC_CSR_ADDR_LEN 12",
        "`define OPCODE_LEN 7",
        "",
    ]

    v_items = [i for i in items if i.target == "v" and not i.hidden]
    if v_items:
        lines.append("// debug print controls")
        for item in v_items:
            val = eff.get(item.name, item.default)
            if val:
                lines.append(f"`define {item.name}")
            else:
                lines.append(f"// `define {item.name}")
        lines.append("")

    with open(VSRC_CONF_H_V, "w") as f:
        f.write("\n".join(lines) + "\n")


def _emit_c(lines: List[str], item: ConfigItem, val: Any):
    """Append one #define or // #define line for a C-side item."""
    if item.type in ("bool", "choice"):
        if val:
            lines.append(f"#define {item.name} 1")
        else:
            lines.append(f"// #define {item.name} 1")
    elif item.type == "int":
        lines.append(f"#define {item.name} {hex(val)}")


# ── Curses TUI ──────────────────────────────────────────────


# sentinel for category separator between groups
_SEP = object()


class _Editor:
    """Inline editing state for int values."""

    __slots__ = ("item", "buf", "cursor")

    def __init__(self, item: ConfigItem, current_val: Any):
        self.item = item
        self.buf = hex(current_val)
        self.cursor = len(self.buf)


class MenuConfig:
    """Curses-based interactive configuration TUI."""

    def __init__(self, items: List[ConfigItem], state: Dict[str, Any]):
        self.items = items
        self.item_map = {i.name: i for i in items}
        self.visible = [i for i in items if not i.hidden]
        self.state = state  # raw user preferences
        self.effective = resolve_effective(items, state)
        self._build_rows()
        self.cursor_row = 0  # index into visible items
        self.scroll = 0  # first visible display row
        self.msg = ""
        self.editor: Optional[_Editor] = None

    # ── row index mapping ──────────────────────────────

    def _build_rows(self):
        """Rebuild flat display-row list from visible items."""
        rows = []
        self._ridx_map = {}  # item name → row index
        last_cat = None
        for item in self.visible:
            if item.category != last_cat:
                if rows:
                    rows.append(_SEP)
                rows.append(item.category)
                last_cat = item.category
            self._ridx_map[item.name] = len(rows)
            rows.append(item)
        self.rows = rows

    @property
    def _cursor_item(self) -> Optional[ConfigItem]:
        count = 0
        for r in self.rows:
            if isinstance(r, ConfigItem):
                if count == self.cursor_row:
                    return r
                count += 1
        return None

    def _cursor_ridx(self) -> int:
        """Display-row index for the cursor item."""
        item = self._cursor_item
        if item:
            return self._ridx_map.get(item.name, 0)
        return 0

    def _dep_depth(self, item: ConfigItem) -> int:
        """How many levels deep in the dependency chain."""
        depth = 0
        cur = item
        while cur.depends and cur.depends in self.item_map:
            depth += 1
            cur = self.item_map[cur.depends]
        return depth

    def _is_dep_disabled(self, item: ConfigItem) -> bool:
        if not item.depends:
            return False
        return not bool(self.effective.get(item.depends, False))

    # ── drawing ────────────────────────────────────────

    def _draw(self, stdscr):
        h, w = stdscr.getmaxyx()

        # shorthand color attrs
        CH = curses.color_pair(1)  # header
        CO = curses.color_pair(2)  # on
        CF = curses.color_pair(3)  # off
        CC = curses.color_pair(4)  # cursor bar
        CD = curses.color_pair(5)  # dependency-disabled
        CI = curses.color_pair(6)  # int value
        CB = curses.color_pair(7)  # help bar

        # title row
        title = " NPC Configuration — menuconfig "
        stdscr.addstr(0, max(0, (w - len(title)) // 2), title, CB | curses.A_BOLD)

        # help bar
        help_txt = (
            " [↑↓/jk] Navigate  [Space] Toggle  [Enter] Edit int  "
            "[s] Save+Quit  [q] Quit "
        )
        stdscr.addstr(h - 1, max(0, (w - len(help_txt)) // 2), help_txt, CB)

        # editing mode overlay
        if self.editor:
            return self._draw_editor(stdscr)

        content_h = h - 4  # rows for the item list
        total_rows = len(self.rows)
        cursor_ridx = self._cursor_ridx()

        # auto-scroll
        if cursor_ridx < self.scroll:
            self.scroll = cursor_ridx
        elif cursor_ridx >= self.scroll + content_h:
            self.scroll = cursor_ridx - content_h + 1

        # status message
        if self.msg:
            try:
                stdscr.addstr(h - 2, 2, self.msg[: w - 4], CD)
            except curses.error:
                pass

        # draw rows
        drawn = 0
        y = 2
        for ri in range(self.scroll, total_rows):
            if drawn >= content_h:
                break
            r = self.rows[ri]

            if r is _SEP:
                try:
                    stdscr.addstr(y, 3, "─" * (w - 6), CH)
                except curses.error:
                    pass
                drawn += 1; y += 1; continue

            if isinstance(r, str):
                try:
                    stdscr.addstr(y, max(0, (w - len(r) - 2) // 2),
                                  f" {r} ", CH | curses.A_BOLD)
                except curses.error:
                    pass
                drawn += 1; y += 1; continue

            # ConfigItem row
            item = r
            item_idx = sum(1 for x in self.rows[:ri]
                           if isinstance(x, ConfigItem))
            is_cursor = (item_idx == self.cursor_row)
            dep_off = self._is_dep_disabled(item)
            indent = "  " * self._dep_depth(item)

            if item.type == "choice":
                sel = bool(self.effective.get(item.name, item.default))
                marker = "(•)" if sel else "( )"
            elif item.type == "int":
                val = self.effective.get(item.name, item.default)
                marker = item.fmt.format(val) if item.fmt else str(val)
            else:
                on = bool(self.effective.get(item.name, item.default))
                marker = "[*]" if on else "[ ]"

            line = f" {indent}{marker}  {item.name:<32s}{item.desc[:w-46]}"

            if is_cursor:
                attr = CC | curses.A_BOLD
            elif dep_off:
                attr = CD
            elif item.type == "int":
                attr = CI
            elif bool(self.effective.get(item.name, item.default)):
                attr = CO
            else:
                attr = CF

            try:
                stdscr.addstr(y, 1, line[: w - 2], attr)
            except curses.error:
                pass
            drawn += 1; y += 1

        # scroll bar
        if total_rows > content_h:
            bar_h = h - 6
            frac = self.scroll / max(1, total_rows - content_h)
            bar_pos = int(frac * bar_h)
            for i in range(bar_h):
                ch = "█" if i == bar_pos else "│"
                try:
                    stdscr.addch(2 + i, w - 1, ch, CH)
                except curses.error:
                    pass

    def _draw_editor(self, stdscr):
        h, w = stdscr.getmaxyx()
        item = self.editor.item
        buf = self.editor.buf
        prompt = f" {item.name}: > {buf}_ "
        try:
            stdscr.addstr(h - 3, 2, prompt[: w - 4], curses.A_REVERSE)
        except curses.error:
            pass
        if item.vmin and item.vmax:
            hint = (f" Range: {item.fmt.format(item.vmin)}"
                    f" ~ {item.fmt.format(item.vmax)}")
            try:
                stdscr.addstr(h - 2, 2, hint[: w - 4], curses.color_pair(6))
            except curses.error:
                pass
        try:
            stdscr.addstr(h - 5, 2, " Enter=confirm  Esc=cancel",
                          curses.color_pair(1))
        except curses.error:
            pass

    # ── input handling ─────────────────────────────────

    def handle_key(self, key: int) -> Optional[bool]:
        """Returns True=save, False=quit, None=continue."""
        if key in (ord("q"), 27):
            return False
        if key == ord("s"):
            return True
        if key in (ord("j"), curses.KEY_DOWN):
            self._move(1)
        elif key in (ord("k"), curses.KEY_UP):
            self._move(-1)
        elif key == ord(" "):
            self._toggle()
        elif key in (curses.KEY_ENTER, 10, 13):
            self._edit_begin()
        elif key == curses.KEY_NPAGE:
            self._move(12)
        elif key == curses.KEY_PPAGE:
            self._move(-12)
        elif key == ord("g"):
            self.cursor_row = 0
        elif key == ord("G"):
            self.cursor_row = len(self.visible) - 1
        return None

    def handle_edit_key(self, key: int) -> Optional[bool]:
        """Handle key while editing an int value."""
        if key == 27:  # Esc
            self.editor = None
            self.msg = "Edit cancelled."
            return None
        if key in (curses.KEY_ENTER, 10, 13):
            return self._edit_commit()
        if key in (curses.KEY_BACKSPACE, 127, 8):
            if self.editor.cursor > 0:
                self.editor.buf = (
                    self.editor.buf[: self.editor.cursor - 1]
                    + self.editor.buf[self.editor.cursor :]
                )
                self.editor.cursor -= 1
            return None
        if key == curses.KEY_LEFT and self.editor.cursor > 0:
            self.editor.cursor -= 1
            return None
        if key == curses.KEY_RIGHT and self.editor.cursor < len(self.editor.buf):
            self.editor.cursor += 1
            return None
        if key == curses.KEY_DC:  # Delete
            if self.editor.cursor < len(self.editor.buf):
                self.editor.buf = (
                    self.editor.buf[: self.editor.cursor]
                    + self.editor.buf[self.editor.cursor + 1 :]
                )
            return None
        if 32 <= key < 127:
            c = chr(key)
            self.editor.buf = (
                self.editor.buf[: self.editor.cursor]
                + c
                + self.editor.buf[self.editor.cursor :]
            )
            self.editor.cursor += 1
        return None

    # ── actions ────────────────────────────────────────

    def _move(self, delta: int):
        self.cursor_row = max(0, min(len(self.visible) - 1,
                                      self.cursor_row + delta))

    def _toggle(self):
        item = self._cursor_item
        if item is None:
            return
        if self._is_dep_disabled(item):
            self.msg = f"{item.name}: dependency not met, cannot toggle."
            return

        if item.type == "bool":
            self.state[item.name] = not bool(
                self.state.get(item.name, item.default))
        elif item.type == "choice":
            group = [i for i in self.items if i.choice_group == item.choice_group]
            for other in group:
                self.state[other.name] = (other.name == item.name)
        elif item.type == "int":
            self._edit_begin()
            return

        self.effective = resolve_effective(self.items, self.state)
        self._build_rows()

    def _edit_begin(self):
        item = self._cursor_item
        if item is None or item.type != "int":
            return
        if self._is_dep_disabled(item):
            self.msg = f"{item.name}: dependency not met."
            return
        cur = self.state.get(item.name, item.default)
        self.editor = _Editor(item, cur)
        self.msg = ""

    def _edit_commit(self) -> Optional[bool]:
        item = self.editor.item
        raw = self.editor.buf.strip()
        try:
            if raw.lower().startswith("0x"):
                val = int(raw, 16)
            else:
                val = int(raw, 0)
        except ValueError:
            self.msg = f"Invalid value: '{raw}'"
            self.editor = None
            return None
        if item.vmin and val < item.vmin:
            self.msg = (f"Too small: {hex(val)}"
                        f" (min {item.fmt.format(item.vmin)})")
            self.editor = None
            return None
        if item.vmax and val > item.vmax:
            self.msg = (f"Too large: {hex(val)}"
                        f" (max {item.fmt.format(item.vmax)})")
            self.editor = None
            return None
        self.state[item.name] = val
        self.effective = resolve_effective(self.items, self.state)
        self.msg = f"Set {item.name} = {hex(val)}"
        self.editor = None
        self._build_rows()
        return None

    # ── main loop ──────────────────────────────────────

    def run(self, stdscr) -> bool:
        """TUI main loop. Returns True if saved, False if quit."""
        curses.curs_set(0)
        stdscr.keypad(True)
        if curses.has_colors():
            curses.start_color()
            curses.use_default_colors()
            curses.init_pair(1, curses.COLOR_CYAN, -1)   # header
            curses.init_pair(2, curses.COLOR_GREEN, -1)   # on
            curses.init_pair(3, curses.COLOR_WHITE, -1)   # off
            curses.init_pair(4, curses.COLOR_BLACK, curses.COLOR_WHITE)  # cursor
            curses.init_pair(5, curses.COLOR_RED, -1)     # dep-disabled
            curses.init_pair(6, curses.COLOR_YELLOW, -1)  # int
            curses.init_pair(7, curses.COLOR_BLACK, curses.COLOR_CYAN)  # bar

        while True:
            try:
                self._draw(stdscr)
                stdscr.refresh()
            except curses.error:
                pass

            try:
                key = stdscr.getch()
            except KeyboardInterrupt:
                return False

            if self.editor:
                result = self.handle_edit_key(key)
            else:
                result = self.handle_key(key)

            if result is not None:
                return result


# ── Entry Point ─────────────────────────────────────────────


def main() -> int:
    # 0. backup
    backup_once()

    # 1. load schema + state
    items = load_items(ITEMS_YAML)
    state = load_state(items)

    # 2. run TUI
    mc = MenuConfig(items, state)
    try:
        saved = curses.wrapper(mc.run)
    except Exception as e:
        # try to restore terminal before printing error
        print(f"\nError: {e}", file=sys.stderr)
        return 1

    # 3. save & generate
    if saved:
        eff = resolve_effective(items, mc.state)
        save_state(mc.state)
        generate_conf_h(eff, items)
        generate_vsrc_conf_h_v(eff, items)
        print()
        print("  Configuration saved → .config.json")
        print(f"  Generated: {CONF_H.name}")
        print(f"  Generated: {VSRC_CONF_H_V.name}")
        print()
        print("  Run 'make' to rebuild with the new configuration.")
    else:
        print()
        print("  Quit without saving.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
