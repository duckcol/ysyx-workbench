#include <common.h>
#include <sys/time.h>

/* for uptime reg */
static uint64_t boot_time = 0;

static uint64_t get_time_internal() {
  struct timeval now;
  gettimeofday(&now, NULL);
  uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
  return us;
}

uint64_t get_time() {
  if (boot_time == 0)
    boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

/* for vga reg */
#define SCREEN_W (MUXDEF(CONFIG_VGA_SIZE_800x600, 800, 400))
#define SCREEN_H (MUXDEF(CONFIG_VGA_SIZE_800x600, 600, 300))

static uint32_t screen_width() {
  // return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).width, SCREEN_W);
  return SCREEN_W;
}

static uint32_t screen_height() {
  // return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).height, SCREEN_H);
  return SCREEN_H;
}

static uint32_t screen_size() {
  return screen_width() * screen_height() * sizeof(uint32_t);
}

static void *vmem = NULL;
static uint32_t vgactl_port_base[2];

#ifdef CONFIG_VGA_SHOW_SCREEN
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static void init_screen() {
  SDL_Window *window = NULL;
  char title[128];
  sprintf(title, "%s-NPC", str(risv32e));
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(
      SCREEN_W * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      SCREEN_H * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)), 0, &window,
      &renderer);
  SDL_SetWindowTitle(window, title);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);
  SDL_RenderPresent(renderer);
}

static inline void update_screen() {
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}
#else
static void init_screen() {}

static inline void update_screen() {
  io_write(AM_GPU_FBDRAW, 0, 0, vmem, screen_width(), screen_height(), true);
}
#endif

void vga_update_screen() {
  // TODO: call `update_screen()` when the sync register is non-zero,
  // then zero out the sync register
  if (vgactl_port_base[1]) {
    update_screen();
    vgactl_port_base[1] = 0;
  }
}

#endif

void init_vga() {
  // screen width and height info stored in low 32 bits of vgactl reg
  vgactl_port_base[0] = (screen_width() << 16) | screen_height();
  vgactl_port_base[1] = 0;

  // vmem is set to draw frames
  vmem = malloc(screen_size());
  if (!vmem)
    Assert(0, "vmem assign error");

  // init SDL windows and set vmem to be 0
  IFDEF(CONFIG_VGA_SHOW_SCREEN, init_screen());
  IFDEF(CONFIG_VGA_SHOW_SCREEN, memset(vmem, 0, screen_size()));
}

uint32_t get_vga_ctl_info(int select) {
  switch (select) {
  case 1:
    //  return width and height
    return vgactl_port_base[0];
    break;
  case 2:
    // return sync
    return vgactl_port_base[1];
    break;
  case 3:
    //  return vmem size
    return screen_size();
    break;
  default:
    Assert(0, "should not get it");
    return 0;
    break;
  }
}

uint32_t get_fb_data(int addr_offset) {
  return *(uint32_t *)((uint8_t *)vmem + addr_offset);
}

void change_vga_sync(uint32_t next_state) { vgactl_port_base[1] = next_state; }

void change_vga_fb(int addr_offset, uint32_t pixels) {
  *(uint32_t *)((uint8_t *)vmem + addr_offset) = pixels;
}

/* init device */
void init_device() {
  Log("device init start");
  IFDEF(CONFIG_HAS_VGA, init_vga());
  Log("device init end");
}
