#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) { panic("Not implemented"); }

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}
int my_itoa(int64_t value, char *buffer, int base) {
  // 处理无效进制
  if (base < 2 || base > 36) {
    *buffer = '\0';
    return 0;
  }

  char *p = buffer;
  uint64_t unsigned_value;
  int is_negative = 0;

  // 处理负数（仅当基数为10时）
  if (value < 0 && base == 10) {
    is_negative = 1;
    unsigned_value = (uint64_t)(-value);
  } else {
    unsigned_value = (uint64_t)value;
  }

  // 处理0的特殊情况
  if (unsigned_value == 0) {
    *p++ = '0';
    *p = '\0';
    return 1;
  }

  // 转换数字（逆序存储）
  char temp[65]; // 足够存储64位整数
  int i = 0;

  while (unsigned_value != 0) {
    int digit = unsigned_value % base;
    temp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    unsigned_value /= base;
  }

  // 添加符号（如果需要）
  if (is_negative) {
    *p++ = '-';
  }

  // 反转数字顺序
  int count = i;
  while (i > 0) {
    *p++ = temp[--i];
  }
  *p = '\0';

  return count + (is_negative ? 1 : 0);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *p = out;

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      if (*fmt == '\0')
        break;

      switch (*fmt) {
      case 'd': {
        int num = va_arg(args, int);
        char num_buf[20];
        int len = my_itoa(num, num_buf, 10);
        memcpy(p, num_buf, len);
        p += len;
        fmt++;
        break;
      }
      case 's': {
        char *s = va_arg(args, char *);
        size_t len = strlen(s);
        memcpy(p, s, len);
        p += len;
        fmt++;
        break;
      }
      case 'c': {
        char c = va_arg(args, int);
        *p++ = c;
        fmt++;
        break;
      }
      // 可以轻松扩展其他格式
      case 'x': { // 十六进制
        unsigned int num = va_arg(args, unsigned int);
        char num_buf[20];
        int len = my_itoa(num, num_buf, 16);
        memcpy(p, num_buf, len);
        p += len;
        fmt++;
        break;
      }
      case 'o': { // 八进制
        unsigned int num = va_arg(args, unsigned int);
        char num_buf[20];
        int len = my_itoa(num, num_buf, 8);
        memcpy(p, num_buf, len);
        p += len;
        fmt++;
        break;
      }
      case '%': {
        *p++ = '%';
        fmt++;
        break;
      }
      default: {
        *p++ = '%';
        *p++ = *fmt++;
        break;
      }
      }
    } else {
      *p++ = *fmt++;
    }
  }

  *p = '\0';
  va_end(args);
  return p - out;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
