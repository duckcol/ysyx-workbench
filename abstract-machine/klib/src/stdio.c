#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// ============ 工具函数 ============
static int my_itoa(int64_t value, char *buffer, int base) {
  if (base < 2 || base > 36) {
    *buffer = '\0';
    return 0;
  }
  char *p = buffer;
  uint64_t unsigned_value;
  int is_negative = 0;

  if (value < 0 && base == 10) {
    is_negative = 1;
    unsigned_value = (uint64_t)(0 - (uint64_t)value);
  } else {
    unsigned_value = (uint64_t)value;
  }

  if (unsigned_value == 0) {
    *p++ = '0';
    *p = '\0';
    return 1;
  }

  char temp[65];
  int i = 0;
  while (unsigned_value != 0) {
    int digit = unsigned_value % base;
    temp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    unsigned_value /= base;
  }
  if (is_negative)
    *p++ = '-';
  while (i > 0)
    *p++ = temp[--i];
  *p = '\0';
  return (int)(p - buffer);
}

static void put_padding(char **p, char *end, char ch, int count) {
  while (count-- > 0 && *p < end) {
    *(*p)++ = ch;
  }
}

// ============ 核心格式化函数 ============

static int vsnprintf_core(char *out, size_t size, const char *fmt, va_list ap,
                          int count_only) {
  char *p = out;
  char *end = (size > 0 && !count_only) ? (out + size - 1) : NULL;
  int total_len = 0;

  while (*fmt) {
    if (*fmt == '%') {
      fmt++; // 跳过 '%'
      if (*fmt == '\0')
        break;

      // --- 1. 解析标志位和宽度 (支持 %02d, %5d 等) ---
      int width = 0;
      int zero_pad = 0;

      // 解析 '0' 标志
      if (*fmt == '0') {
        zero_pad = 1;
        fmt++;
      }

      // 解析数字宽度 (如 2, 5, 10)
      while (*fmt >= '0' && *fmt <= '9') {
        width = width * 10 + (*fmt - '0');
        fmt++;
      }

      // 跳过长度修饰符 (l, h, z 等，32 位环境下暂时忽略，统一按 int 处理)
      if (*fmt == 'l' || *fmt == 'h' || *fmt == 'z') {
        fmt++;
        if (*(fmt - 1) == 'l' && *fmt == 'l')
          fmt++; // 跳过 ll
      }

      // --- 2. 根据转换符处理 ---
      // 此时 *fmt 应该是 d, x, s, c, % 等
      char spec = *fmt;

      // 默认先推进 fmt，除非下面 break 前需要特殊处理
      fmt++;

      switch (spec) {
      case 'd':
      case 'i': {
        // ✅ 修复：32 位环境用 int 读取，再转 int64_t 给 itoa
        int val = va_arg(ap, int);
        char num_buf[25];
        int num_len = my_itoa((int64_t)val, num_buf, 10);

        int sign_len = (num_buf[0] == '-') ? 1 : 0;
        int content_len = num_len;
        int pad_len = (width > content_len) ? (width - content_len) : 0;

        total_len += (width > 0) ? width : content_len;

        if (!count_only && end != NULL) {
          if (zero_pad && !sign_len)
            put_padding(&p, end, '0', pad_len);
          if (sign_len && p < end) {
            *p++ = '-';
            if (zero_pad)
              put_padding(&p, end, '0', pad_len);
          }
          char *num_start = num_buf + sign_len;
          int digits_len = num_len - sign_len;
          for (int i = 0; i < digits_len && p < end; i++)
            *p++ = num_start[i];
          if (!zero_pad)
            put_padding(&p, end, ' ', pad_len);
        }
        break;
      }

      case 'u': {
        unsigned int val = va_arg(ap, unsigned int);
        char num_buf[25];
        int num_len = my_itoa((int64_t)val, num_buf, 10);
        int pad_len = (width > num_len) ? (width - num_len) : 0;
        total_len += (width > 0) ? width : num_len;
        if (!count_only && end != NULL) {
          if (zero_pad)
            put_padding(&p, end, '0', pad_len);
          for (int i = 0; i < num_len && p < end; i++)
            *p++ = num_buf[i];
          if (!zero_pad)
            put_padding(&p, end, ' ', pad_len);
        }
        break;
      }

      case 'x':
      case 'X': {
        unsigned int val = va_arg(ap, unsigned int);
        char num_buf[20];
        int num_len = my_itoa((int64_t)val, num_buf, 16);
        if (spec == 'X') {
          for (int i = 0; i < num_len; i++)
            if (num_buf[i] >= 'a' && num_buf[i] <= 'f')
              num_buf[i] -= 'a' - 'A';
        }
        int pad_len = (width > num_len) ? (width - num_len) : 0;
        total_len += (width > 0) ? width : num_len;
        if (!count_only && end != NULL) {
          if (zero_pad)
            put_padding(&p, end, '0', pad_len);
          for (int i = 0; i < num_len && p < end; i++)
            *p++ = num_buf[i];
          if (!zero_pad)
            put_padding(&p, end, ' ', pad_len);
        }
        break;
      }

      case 'o': {
        unsigned int val = va_arg(ap, unsigned int);
        char num_buf[25];
        int num_len = my_itoa((int64_t)val, num_buf, 8);
        int pad_len = (width > num_len) ? (width - num_len) : 0;
        total_len += (width > 0) ? width : num_len;
        if (!count_only && end != NULL) {
          if (zero_pad)
            put_padding(&p, end, '0', pad_len);
          for (int i = 0; i < num_len && p < end; i++)
            *p++ = num_buf[i];
          if (!zero_pad)
            put_padding(&p, end, ' ', pad_len);
        }
        break;
      }

      case 's': {
        char *s = va_arg(ap, char *);
        int len = 0;
        if (s) {
          while (s[len])
            len++;
        }
        total_len += len;
        if (!count_only && end != NULL && s) {
          while (*s && p < end)
            *p++ = *s++;
        }
        break;
      }

      case 'c': {
        int val = va_arg(ap, int); // char 提升为 int
        total_len++;
        if (!count_only && end != NULL && p < end)
          *p++ = (char)val;
        break;
      }

      case '%': {
        total_len++;
        if (!count_only && end != NULL && p < end)
          *p++ = '%';
        break;
      }

      default: {
        // 未知格式符，原样输出 % 和字符
        total_len += 2;
        if (!count_only && end != NULL) {
          if (p < end)
            *p++ = '%';
          if (p < end)
            *p++ = spec;
        }
        break;
      }
      }
    } else {
      total_len++;
      if (!count_only && end != NULL && p < end)
        *p++ = *fmt;
      fmt++;
    }
  }

  if (!count_only && end != NULL && p < end + 1)
    *p = '\0';
  return total_len;
}

// ============ 公开接口 ============
int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf_core(out, (size_t)-1, fmt, ap, 0);
}
int vsnprintf(char *out, size_t size, const char *fmt, va_list ap) {
  return vsnprintf_core(out, size, fmt, ap, 0);
}

int printf(const char *fmt, ...) {
  char buf[256];
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  putstr(buf);
  return len;
}
int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(out, n, fmt, args);
  va_end(args);
  return ret;
}
int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vsprintf(out, fmt, args);
  va_end(args);
  return ret;
}

#endif
