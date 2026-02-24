#ifndef AVL_H
#define AVL_H

#include <z_array.h>
#include <z_compare.h>
#include <z_heap.h>
#include <stdbool.h>

typedef struct {
  void *key;
  void *value;
} Z_Key_Value;

Z_DEFINE_ARRAY(Z_Key_Value_Array, Z_Key_Value);

typedef struct Z_Avl_Node {
  unsigned int left;
  unsigned int right;
  void *key;
  void *value;
  int height;
} Z_Avl_Node;

Z_DEFINE_ARRAY(Z_Avl_Node_Array, Z_Avl_Node);
Z_DEFINE_ARRAY(Z_Free_List, unsigned int);

typedef struct {
  Z_Avl_Node_Array nodes;
  Z_Free_List free_list;
  unsigned int root;
  Z_Compare_Fn compare_keys;
} Z_Avl_Tree;

Z_Avl_Tree z_avl_tree_new(Z_Heap *heap, Z_Compare_Fn compare_keys);
void *z__avl_try_get(const Z_Avl_Tree *tree, const void *key, const void *fallback);
void *z__avl_get(const Z_Avl_Tree *tree, const void *key);
bool z__avl_contains(const Z_Avl_Tree *tree, void *key);
Z_Key_Value z__avl_put(Z_Avl_Tree *tree, void *key, void *value);
Z_Key_Value z__avl_remove(Z_Avl_Tree *tree, void *key);
Z_Key_Value_Array z__avl_to_array(Z_Heap *heap, const Z_Avl_Tree *tree);
void z__avl_print(const Z_Avl_Tree *root, Z_Print_Fn print_key, Z_Print_Fn print_value);
void z__avl_print_hierarchy(Z_Avl_Tree *tree, Z_Print_Fn print_key, Z_Print_Fn print_value, const char *separator);

#endif
