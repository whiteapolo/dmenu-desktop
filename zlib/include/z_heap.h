#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
  uintptr_t *ptr;
  size_t occupied;
  size_t capacity;
} Z_Ptr_Table;

typedef Z_Ptr_Table Z_Heap;

typedef void (*Z_Free_Fn)(Z_Heap *, void *);

#define Z_Heap_Auto __attribute__((cleanup(z_heap_free_all))) Z_Heap

void *z_heap_malloc(Z_Heap *heap, size_t size);
void *z_heap_calloc(Z_Heap *heap, size_t size);
void *z_heap_realloc(Z_Heap *heap, void *ptr, size_t new_size);
void z_heap_free(Z_Heap *heap, void *ptr);
void z_heap_free_all(Z_Heap *heap);
void z_heap_reset(Z_Heap *heap);

#endif
