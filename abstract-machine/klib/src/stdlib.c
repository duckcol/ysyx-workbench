#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }

int abs(int x) { return (x < 0 ? -x : x); }

int atoi(const char *nptr) {
  int x = 0;
  while (*nptr == ' ') {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr++;
  }
  return x;
}

size_t heap_top_position = 0;
void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  // panic("Not implemented");
  void *heap_top_ptr = heap.start;
  size = (size_t)ROUNDUP(size, 4); // the malloc mem will be 4-byte aligned

  if (IN_RANGE((void *)((size_t)heap_top_ptr + size + heap_top_position),
               heap)) {
    heap_top_ptr += heap_top_position;
    heap_top_position += size;
    return heap_top_ptr;
  } else {
    return NULL;
  }
#endif
  return NULL;
}

void free(void *ptr) {
  if (ptr == NULL) {
    return;
  }
}

#endif
