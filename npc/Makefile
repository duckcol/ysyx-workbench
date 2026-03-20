TOPNAME = antpc

NPC_DIR := $(YSYX_HOME)/npc
CPP_CODE := $(shell find csrc -type f \( -name "*.cpp" -o -name "*.c" \))
VLOG_CODE := $(sort $(shell find vsrc -type f -name "*.v"))
WAVEFORM_FILE=$(wildcard *.fst *.vcd)

# IMG_PATH := $(YSYX_HOME)/am-kernels/tests/cpu-tests/build
# IMG := $(wildcard image/*.bin)

VERILATOR = verilator
VERILATOR_CFLAGS += --cc --MMD --build -j 0 \
										--x-assign fast \
										--x-initial fast \
										-O3 --noassert \
										--trace-fst
#-Wall

BUILD_DIR = $(NPC_DIR)/build
OBJ_DIR = $(BUILD_DIR)
BIN = $(BUILD_DIR)/V$(TOPNAME)

DEFAULT_IMG = $(NPC_DIR)/dummy-riscv32e-npc.bin
IMG ?= $(NPC_DIR)/dummy-riscv32e-npc.bin

DEFAULT_LOG = $(NPC_DIR)/default-npc-log.txt
LOG ?= $(NPC_DIR)/default-npc-log.txt

DEFAULT_ELF = $(NPC_DIR)/dummy-riscv32e-npc.elf
ELF ?= $(NPC_DIR)/dummy-riscv32e-npc.elf

DEFAULT_DIFF_SO = $(YSYX_HOME)/nemu/build/riscv32-nemu-interpreter-so
DIFF_SO ?= $(YSYX_HOME)/nemu/build/riscv32-nemu-interpreter-so

BIN_VERIALATOR_OUTPUT ?= $(NPC_DIR)/verilator-compilation-output

CXXFLAGS += -DTOP_NAME="\"V$(TOPNAME)\"" -I$(NPC_DIR)/csrc/include/
LDFLAGS += -lreadline
# to link lib llvm
LLVM_RAW_FLAGS := $(shell llvm-config --cxxflags)
# remove some old macros that will cause error
LLVM_SAFE_FLAGS := $(filter-out -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS,$(LLVM_RAW_FLAGS))
CXXFLAGS += $(LLVM_SAFE_FLAGS) -fPIE
LIBS += $(shell llvm-config --libs)
LDFLAGS += $(LIBS)

# verilog display macros
VERILOG_DEBUG_MARCO = IFU IDU ALUOPCTRL ALUINCTRL ALU TARGET_ADDR_CTRL
VERILATOR_DEBUG_MACRO = $(addprefix -DDEBUG_, $(VERILOG_DEBUG_MARCO))

all: $(BIN)
	$(shell mkdir -p $(BUILD_DIR))

$(BUILD_DIR):
	$(shell mkdir -p $@)

$(BIN): $(VLOG_CODE) $(CPP_CODE) | $(BUILD_DIR)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $(VLOG_CODE) $(CPP_CODE)\
		--Mdir $(BUILD_DIR) \
		$(VERILATOR_DEBUG_MACRO) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		--exe > $(BIN_VERIALATOR_OUTPUT)

# Command to run npc
# "override" is used to prevent the following lines with ARGS being ignored
override ARGS ?= \
	--log=$(shell realpath $(DEFAULT_LOG)) \
	--ftrace=$(shell realpath $(DEFAULT_ELF)) \
	--diff=$(shell realpath $(DEFAULT_DIFF_SO))

NPC_EXEC := $(shell realpath $(BIN)) $(ARGS) $(shell realpath $(IMG))

run-env: $(BIN)

run: run-env
	@if [ "$(IMG)" = "$(DEFAULT_IMG)" ]; then \
		echo USING DEFAULT_IMG; \
	fi
	@# print full Command to exec npc
	@echo "=== Full Command Breakdown ==="
	@# 打印可执行文件
	@printf "  [EXE] %s\n" "$(shell realpath $(BIN))"
	@# 遍历打印参数 (假设 ARGS 由空格分隔)
	@for arg in $(ARGS); do \
		printf "  [ARG] %s\n" "$$arg"; \
	done
	@# 打印镜像文件
	@printf "  [IMG] %s\n" "$(shell realpath $(IMG))"
	@echo "=============================="
	@$(NPC_EXEC)

gdb: run-env
	gdb $(BIN)

check_waveform: $(WAVEFORM_FILE)
	@gtkwave $(WAVEFORM_FILE) --dark

file_comfirm:
	@echo "=== Project Configuration ==="
	@echo "npc dir path is $(NPC_DIR)"
	@echo "cpp code is $(CPP_CODE)"
	@echo "verilog code is $(VLOG_CODE)"
	@echo "waveform file is $(WAVEFORM_FILE)"
	@echo "img src path is $(IMG_PATH)"
	@echo "img: $(IMG)"
	@echo ""
	@echo "=== Code Statistics ==="
	@# 统计 C++ 代码行数
	@echo "C++ Lines: $$(wc -l $(CPP_CODE) | tail -n 1 | awk '{print $$1}')"
	@# 统计 Verilog 代码行数
	@echo "Verilog Lines: $$(wc -l $(VLOG_CODE) | tail -n 1 | awk '{print $$1}')"
	@echo ""

verilog_lint: $(VLOG_CODE)
	@echo "=== Verilator Linter Result ==="
	@verilator --lint-only -Wall $(VLOG_CODE)

sim: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$^

.PYONY: clean sim file_comfirm run gdb get-img

clean:
	rm -rf $(BUILD_DIR) obj_dir *.fst* image *log.txt

include ../Makefile
