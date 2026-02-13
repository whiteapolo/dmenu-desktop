// #ifndef AVL_H
// #define AVL_H

// #include <z_array.h>
// #include <z_compare.h>
// #include <z_heap.h>
// #include <stdbool.h>

// typedef struct {
//   void *key;
//   void *value;
// } Z_Key_Value;

// Z_DEFINE_ARRAY(Z_Key_Value_Array, Z_Key_Value);

// typedef struct Z_Avl_Node {
//   struct Z_Avl_Node *left;
//   struct Z_Avl_Node *right;
//   void *key;
//   void *value;
//   char height;
// } Z_Avl_Node;

// Z_Avl_Node *z__avl_new(
//     Z_Heap *heap,
//     void *key,
//     void *value
// );

// Z_Avl_Node *z__avl_get_min(
//     Z_Avl_Node *root
// );

// void *z__avl_try_get(
//   Z_Avl_Node *root,
//   const void *key,
//   Z_Compare_Fn compare_keys,
//   const void *fallback
// );

// void *z__avl_get(
//   Z_Avl_Node *root,
//   const void *key,
//   Z_Compare_Fn compare_keys
// );

// bool z__avl_has(
//     Z_Avl_Node *root,
//     void *key,
//     Z_Compare_Fn compare_keys
// );

// Z_Key_Value z__avl_put(
//     Z_Heap *heap,
//     Z_Avl_Node **root,
//     void *key,
//     void *value,
//     Z_Compare_Fn compare_keys
// );

// Z_Key_Value z__avl_remove(
//     Z_Heap *heap,
//     Z_Avl_Node **root,
//     void *key,
//     Z_Compare_Fn compare_keys
// );

// Z_Key_Value_Array z__avl_to_array(
//   Z_Heap *heap,
//   const Z_Avl_Node *root
// );

// void z__avl_print(
//     Z_Avl_Node *root,
//     Z_Print_Fn print_key,
//     Z_Print_Fn print_value
// );

// void z__avl_print_hierarchy(
//     Z_Avl_Node *root,
//     Z_Print_Fn print_key,
//     Z_Print_Fn print_value,
//     const char *separator
// );

// #endif
