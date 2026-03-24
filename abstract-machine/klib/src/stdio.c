#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

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
    unsigned_value =
        (uint64_t)(0 - (uint64_t)value); // ✅ 安全：先转 uint64_t 再取负
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

// ============ 改进版：支持"计数模式" ============
static int vsnprintf_core(char *out, size_t size, const char *fmt, va_list ap,
                          int count_only) {
  char *p = out;
  char *end = (size > 0 && !count_only) ? (out + size - 1) : NULL;
  int total_len = 0; // 记录本应写入的总长度

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      if (*fmt == '\0')
        break;

      switch (*fmt) {
      case 'd': {
        int num = va_arg(ap, int);
        char num_buf[20];
        int len = my_itoa(num, num_buf, 10);
        total_len += len;
        if (!count_only && end != NULL) {
          memcpy(p, num_buf, len);
          p += len;
        }
        fmt++;
        break;
      }
      case 's': {
        char *s = va_arg(ap, char *);
        if (s) {
          int len = strlen(s);
          total_len += len;
          if (!count_only && end != NULL) {
            memcpy(p, s, len);
            p += len;
          }
        }
        fmt++;
        break;
      }
      case 'c': {
        total_len++;
        if (!count_only && end != NULL) {
          *p++ = va_arg(ap, int);
          fmt++;
          break;
        }
      }
      case 'x': {
        unsigned int num = va_arg(ap, unsigned int);
        char num_buf[20];
        int len = my_itoa(num, num_buf, 16);
        total_len += len;
        if (!count_only && end != NULL) {
          memcpy(p, num_buf, len);
          p += len;
        }
        fmt++;
        break;
      }
      case 'o': {
        unsigned int num = va_arg(ap, unsigned int);
        char num_buf[20];
        int len = my_itoa(num, num_buf, 8);
        total_len += len;
        if (!count_only && end != NULL) {
          memcpy(p, num_buf, len);
          p += len;
        }
        fmt++;
        break;
      }
      case '%': {
        total_len++;
        if (!count_only && p < end)
          *p++ = '%';
        fmt++;
        break;
      }
      // ... 其他 case 类似，都要累加 total_len ...
      default: {
        total_len += 2; // % + 未知字符
        if (!count_only) {
          if (p < end)
            *p++ = '%';
          if (p < end)
            *p++ = *fmt;
        }
        fmt++;
        break;
      }
      }
    } else {
      total_len++;
      if (!count_only && p < end)
        *p++ = *fmt;
      fmt++;
    }
  }

  if (!count_only && out)
    *p = '\0';
  return total_len; // ✅ 返回"本应写入的长度"，符合 glibc 语义
}

// vsprintf: 无边界检查，直接调用核心（count_only=0, size=极大值）
int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf_core(out, (size_t)-1, fmt, ap, 0);
}

// vsnprintf: 带边界检查
int vsnprintf(char *out, size_t size, const char *fmt, va_list ap) {
  if (size == 0)
    return vsnprintf_core(out, size, fmt, ap, 1);
  return vsnprintf_core(out, size, fmt, ap, 0);
}

// printf: 复用 vsnprintf
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
