#ifndef ARRAY_H
#define ARRAY_H

#include <string.h>
#include <internal/z_config.h>

#define Z_DEFINE_ARRAY(identifier, element_type) \
typedef struct {                                 \
  Z_Heap *heap;                        \
  element_type *ptr;                             \
  size_t length;                                 \
  size_t capacity;                               \
} identifier

#define z_array_new(heap_ptr, type) ((type){ .heap = heap_ptr, .ptr = NULL, .length = 0, .capacity = 0 })

#define z_array_ensure_capacity(array_ptr, needed)                                                                               \
  do {                                                                                                                           \
    if ((array_ptr)->capacity < (needed)) {                                                                                      \
      size_t new_capacity = z__calculate_new_capacity(array_ptr, needed);                                                                \
      (array_ptr)->ptr = z_heap_realloc((array_ptr)->heap, (array_ptr)->ptr, sizeof(*(array_ptr)->ptr) * new_capacity);          \
      (array_ptr)->capacity = new_capacity;                                                                                      \
    }                                                                                                                            \
  } while (0)

#define z_array_push(array_ptr, element)                           \
  do {                                                             \
    z_array_ensure_capacity(array_ptr, (array_ptr)->length + 1);   \
    (array_ptr)->ptr[(array_ptr)->length++] = element;             \
  } while (0)

#define z_array_push_array(dest_ptr, source_ptr)          \
  do {                                                    \
    for (size_t i = 0; i < (source_ptr)->length; i++) {   \
      z_array_push(dest_ptr, (source_ptr)->ptr[i]);       \
    }                                                     \
  } while (0)

#define z_array_peek(array_ptr)   ((array_ptr)->ptr[(array_ptr)->length - 1])
#define z_array_pop(array_ptr)    ((array_ptr)->ptr[--(array_ptr)->length])

#define z_array_zero_terminate(array_ptr)                                           \
  do {                                                                              \
    z_array_ensure_capacity(array_ptr, (array_ptr)->length + 1);                    \
    memset(&(array_ptr)->ptr[(array_ptr)->length], 0, sizeof(*(array_ptr)->ptr));   \
  } while (0)


#define z__calculate_new_capacity(array_ptr, needed)           \
(                                                              \
  (needed) > ((array_ptr)->capacity * Z_BUFFER_GROWTH_FACTOR)  \
    ? (needed)                                                 \
    : ((array_ptr)->capacity * Z_BUFFER_GROWTH_FACTOR)         \
)

#endif