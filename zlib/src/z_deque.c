#include <z_deque.h>
#include <z_compare.h>
#include <internal/z_config.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <internal/z_math.h>

Z_Deque z_deque_new(Z_Heap *heap)
{
  Z_Deque deque = {
    .heap = heap,
    .capacity = 0,
    .size = 0,
    .front = 0,
    .rear = 0,
    .ptr = NULL,
  };

  return deque;
}

size_t z_deque_size(const Z_Deque *deque)
{
  return deque->size;
}

size_t z__circular_buffer_next_index(size_t size, size_t i)
{
  return (i + 1) % size;
}

size_t z__circular_buffer_previous_index(size_t size, size_t i)
{
  return i == 0 ? size - 1 : i - 1;
}

bool z__deque_is_index_inside(const Z_Deque *deque, size_t i)
{
  if (deque->front < deque->rear) {
    return deque->front <= i && deque->rear <= i;
  }

  return deque->front <= i || i <= deque->rear;
}

void z__deque_debug_print(const Z_Deque *deque, Z_Print_Fn print_element)
{
  if (deque->capacity == 0) {
    printf("[]\n");
    return;
  }

  printf("[ ");
  for (size_t i = 0; i < deque->capacity; i++) {
    if (z__deque_is_index_inside(deque, i)) {
      print_element(deque->ptr[i]);
    } else {
      printf("- ");
    }
  }
  printf("]\n");
}

void z__deque_ensure_capacity(Z_Deque *deque, size_t needed)
{
  if (deque->capacity >= needed) {
    return;
  }

  size_t new_capacity = z__max_size_t(needed, deque->capacity * Z_BUFFER_GROWTH_FACTOR);
  size_t old_capacity = deque->capacity;
  deque->ptr = z_heap_realloc(deque->heap, deque->ptr, sizeof(void *) * new_capacity);
  deque->capacity = new_capacity;

  if (deque->front > deque->rear) {
    size_t front_offset = new_capacity - old_capacity;
    void **old_front = deque->ptr + deque->front;
    void **new_front = old_front + front_offset;
    memmove(new_front, old_front, sizeof(void *) * (front_offset));

    deque->front += front_offset;
  }
}

void *z_deque_at(const Z_Deque *deque, size_t i)
{
  assert(i < deque->size);
  size_t wrapped_index = (deque->front + i) % deque->capacity;
  return deque->ptr[wrapped_index];
}

void z_deque_push_back(Z_Deque *deque, void *element)
{
  z__deque_ensure_capacity(deque, deque->size + 1);
  deque->rear = z__circular_buffer_next_index(deque->capacity, deque->rear);
  deque->ptr[deque->rear] = element;
  deque->size++;
}

void z_deque_push_front(Z_Deque *deque, void *element)
{
  z__deque_ensure_capacity(deque, deque->size + 1);
  deque->front = z__circular_buffer_previous_index(deque->capacity, deque->front);
  deque->ptr[deque->front] = element;
  deque->size++;
}

void *z_deque_pop_back(Z_Deque *deque)
{
  void *element = deque->ptr[deque->rear];
  deque->rear = z__circular_buffer_previous_index(deque->capacity, deque->rear);
  deque->size--;
  return element;
}

void *z_deque_pop_front(Z_Deque *deque)
{
  void *element = deque->ptr[deque->front];
  deque->front = z__circular_buffer_next_index(deque->capacity, deque->front);
  deque->size--;
  return element;
}
