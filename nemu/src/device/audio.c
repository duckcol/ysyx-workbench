/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <SDL2/SDL.h>
#include <common.h>
#include <device/map.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

// AUDIO_CONFIG, RD, bool present; int bufsize
// AUDIO_CTRL,   WR, int freq, channels, samples
// AUDIO_STATUS, RD, int count
// AUDIO_PLAY,   WR, Area buf

// int rp = 0;
static void audio_play(void *userdata, uint8_t *stream, int len) {
  SDL_memset(stream, 0, len);
  if (audio_base[reg_count] > len){
    SDL_memcpy(stream, sbuf, len);
    audio_base[reg_count] = audio_base[reg_count] - len;
    /*去除掉已经SDL缓冲区的内容*/
    for (uint32_t i = 0; i < audio_base[reg_count]; i++)
      sbuf[i] = sbuf[len + i];
  } else {
    SDL_memcpy(stream, sbuf, audio_base[reg_count]);
    SDL_memset(stream + audio_base[reg_count], 0, len - audio_base[reg_count]);
    audio_base[reg_count] = 0;
  }
}

static void init_SDL_audio() {
  SDL_AudioSpec s = {};
  s.freq = audio_base[reg_freq];
  s.format = AUDIO_S16SYS;
  s.channels = audio_base[reg_channels];
  s.samples = audio_base[reg_samples];
  s.callback = audio_play;
  s.userdata = NULL;

  printf("freq=" FMT_WORD " channels=" FMT_WORD" samples=" FMT_WORD"\n", audio_base[reg_freq], audio_base[reg_channels], audio_base[reg_samples]);
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&s, NULL);
  SDL_PauseAudio(0);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  switch (offset) {
    case 0:
    case 4:
    case 8:
      // write in reg_freq, reg_channels and reg_samples
      break;
    case 12:
      // return value of reg_sbuf_size
      // read only
      assert(!is_write);
      break;
    case 16:
      // set reg_init to init SDL Audio system
      if (audio_base[reg_init] == 1)
        init_SDL_audio();
      break;
    case 20:
      // reg_count will count down to 0 after init
      // read only
      // assert(!is_write);
      break;
    default:
      Assert(1, "audio_base offset error");
      break;
  }
}

void init_audio() {
  // alloc uint32_t audio_base[6] to audio_base, each reg takes one:
  // 0:reg_freq; 1:reg_channels; 2:reg_samples;
  // 3:reg_sbuf_size; 4:reg_init; 5:reg_count;
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size,
              audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size,
               audio_io_handler);
#endif

  // put audio data into sbuf for device to read and play
  // reg_count is used to log already uesd sbuf sizes
  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
