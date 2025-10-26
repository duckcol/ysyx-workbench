#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;
  while (*(s + i) != '\0') {
    i++;
  }
  return i;
}

char *strcpy(char *dst, const char *src) { panic("Not implemented"); }

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) { panic("Not implemented"); }

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
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) { panic("Not implemented"); }

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) { panic("Not implemented"); }

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
