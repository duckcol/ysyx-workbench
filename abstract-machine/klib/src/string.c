#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;
  while (*(s + i) != '\0') {
    i++;
  }
  return i;
}
size_t strnlen(const char *s, size_t maxlen) {
  if (strlen(s) <= maxlen)
    return strlen(s);
  else
    return maxlen;
}

void *mempcpy(void *out, const void *in, size_t n);
char *stpcpy(char *dst, const char *src) {
  char *p;

  p = mempcpy(dst, src, strlen(src));
  *p = '\0';

  return p;
}
char *strcpy(char *dst, const char *src) {
  stpcpy(dst, src);
  return dst;
}

char *stpncpy(char *dst, const char *src, size_t dsize) {
  size_t dlen;
  dlen = strnlen(src, dsize);
  return memset(mempcpy(dst, src, dlen), 0, dsize - dlen);
}

char *strncpy(char *dst, const char *src, size_t n) {
  stpncpy(dst, src, n);
  return dst;
}

char *strcat(char *dst, const char *src) {
  stpcpy(dst + strlen(dst), src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  /*  compare the first difference byte (or char)
   *  in the same position of s1 and s2
   */
  for (const unsigned char *p1 = (unsigned char *)s1, *p2 = (unsigned char *)s2;
       *p1 && *p2; p1++, p2++) {
    if (*p1 != *p2) {
      return (int)(*p1 - *p2);
    }
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (*(unsigned char *)(s1 + i) != *(unsigned char *)(s2 + i))
      return (int)(*(unsigned char *)(s1 + i) - *(unsigned char *)(s2 + i));
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  for (size_t i = 0; i < n; i++) {
    *((unsigned char *)s + i) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char temp[n];
  memcpy(temp, src, n);
  return memcpy(dst, temp, n);
}

void *memcpy(void *out, const void *in, size_t n) {
  for (size_t i = 0; i < n; i++) {
    *(unsigned char *)(out + i) = *(unsigned char *)(in + i);
  }
  return out;
}

void *mempcpy(void *out, const void *in, size_t n) {
  return (memcpy(out, in, n) + n);
}

int memcmp(const void *s1, const void *s2, size_t n) {
  /*  compare the first difference byte (or char)
   *  in the same position of s1 and s2 in s1+n or s2+n
   */
  unsigned char *p1, *p2;
  for (size_t i = 0; i < n; i++) {
    p1 = (unsigned char *)s1 + i;
    p2 = (unsigned char *)s2 + i;
    if (*p1 != *p2)
      return (int)(*p1 - *p2);
  }
  return 0;
}

#endif
