#ifndef DEQUE_H
#define DEQUE_H

#include <stdlib.h>
#include <z_heap.h>

typedef struct {
  Z_Heap *heap;
  void **ptr;
  size_t size;
  size_t capacity;
  size_t front;
  size_t rear;
} Z_Deque;

Z_Deque z_deque_new(Z_Heap *heap);
size_t z_deque_size(const Z_Deque *deque);
void *z_deque_at(const Z_Deque *deque, size_t i);
void z_deque_push_back(Z_Deque *deque, void *element);
void z_deque_push_front(Z_Deque *deque, void *element);
void *z_deque_pop_back(Z_Deque *deque);
void *z_deque_pop_front(Z_Deque *deque);

#endif