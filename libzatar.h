/*
 *           --Libzatar--
 *
 *  #define LIBZATAR_IMPLEMENTATION
 */
#ifndef LIBZATAR_H
#define LIBZATAR_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   util header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

#define Z_DEFAULT_GROWTH_RATE 2

#define CALL_F_IF_NOT_NULL(f, ...) if (f) f(__VA_ARGS__)

int z_in_range(int min, int val, int max);
int z_get_file_size(FILE *fp);
int z_get_fmt_size(const char *fmt, ...);
int z_get_fmt_size_va(const char *fmt, va_list ap);
void *z_memdup(const void *mem, const size_t size);
void z_swap(void *a, void *b, const size_t size);
int z_max(int a, int b);
int z_min(int a, int b);
int z_max3(int a, int b, int c);
int z_min3(int a, int b, int c);

int z_print_error(const char *fmt, ...);
int z_print_info(const char *fmt, ...);
int z_print_warning(const char *fmt, ...);

#define z_da_ensure_capacity(da, cap)                                                  \
    do {                                                                               \
        if ((da)->capacity < (cap)) {                                                  \
            int new_capacity = z_max((cap), (da)->capacity * Z_DEFAULT_GROWTH_RATE);   \
            (da)->capacity = new_capacity;                                             \
            (da)->ptr = realloc((da)->ptr, sizeof((da)->ptr[0]) * (da)->capacity);     \
        }                                                                              \
    } while (0)

#define z_da_append(da, item)                      \
    do {                                           \
        z_da_ensure_capacity((da), (da)->len + 1); \
        (da)->ptr[(da)->len++] = (item);           \
    } while (0)

#define z_da_peek(da) ((da)->ptr[(da)->len - 1])
#define z_da_pop(da) ((da)->ptr[--(da)->len])

#define z_da_null_terminate(da)                                \
    do {                                                       \
        z_da_ensure_capacity((da), (da)->len + 1);             \
        memset(&(da)->ptr[(da)->len], 0, sizeof(*(da)->ptr));  \
    } while (0)

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   cursor header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

#define CTRL_KEY(k)             ((k) & 0x1f)

#define Z_COLOR_RESET           "\033[0m"

#define Z_COLOR_RED             "\033[0;31m"
#define Z_COLOR_GREEN           "\033[0;32m"
#define Z_COLOR_YELLOW          "\033[0;33m"
#define Z_COLOR_BLUE            "\033[0;34m"
#define Z_COLOR_MAGENTA         "\033[0;35m"
#define Z_COLOR_CYAN            "\033[0;36m"
#define Z_COLOR_WHITE           "\033[0;37m"
#define Z_COLOR_GRAY            "\033[0;90m"

#define Z_COLOR_BOLD_RED        "\033[1;91m"
#define Z_COLOR_BOLD_GREEN      "\033[1;92m"
#define Z_COLOR_BOLD_YELLOW     "\033[1;93m"
#define Z_COLOR_BOLD_BLUE       "\033[1;94m"
#define Z_COLOR_BOLD_MAGENTA    "\033[1;95m"
#define Z_COLOR_BOLD_CYAN       "\033[1;96m"
#define Z_COLOR_BOLD_WHITE      "\033[1;97m"
#define Z_COLOR_BOLD_GRAY       "\033[1;90m"

#define Z_KEY_EMPTY         999
#define Z_KEY_ARROW_LEFT    1000
#define Z_KEY_ARROW_RIGHT   1001
#define Z_KEY_ARROW_UP      1002
#define Z_KEY_ARROW_DOWN    1003
#define Z_KEY_PAGE_UP       1007
#define Z_KEY_PAGE_DOWN     1008
#define Z_KEY_DELETE        1004
#define Z_KEY_HOME          1005
#define Z_KEY_END           1006

typedef enum {
    Z_CURSOR_STYLE_BLOCK_STEADY       = 0,
    Z_CURSOR_STYLE_BLOCK_BLINKING     = 1,
    Z_CURSOR_STYLE_UNDERLINE_BLINKING = 3,
    Z_CURSOR_STYLE_UNDERLINE_STEADY   = 4,
    Z_CURSOR_STYLE_BEAM_STEADY        = 6,
    Z_CURSOR_STYLE_BEAM_BLINKING      = 5,
} Z_CURSOR_STYLE;

#define z_set_cursor_style(style)         printf("\033[%dq", (int)(style))
#define z_disable_line_wrap()             printf("\033[?7l")
#define z_enbale_line_wrap()              printf("\033[?7h")
#define z_hide_cursor()                   printf("\033[?25l")
#define z_show_cursor()                   printf("\033[?25h")
#define z_set_cursor_pos(x, y)            printf("\033[%d;%dH", (y), (x))
#define z_set_cursor_x(x)                 printf("\033[%dG", (x))
#define z_cursor_up(n)                    printf("\033[%dA", (n))
#define z_cursor_down(n)                  printf("\033[%dB", (n))
#define z_cursor_right(n)                 printf("\033[%dC", (n))
#define z_cursor_left(n)                  printf("\033[%dD", (n))
// unsupported on some terminals
#define z_save_cursor_pos()               printf("\033[s")
#define z_restore_cursor_pos()            printf("\033[u")
#define z_enter_alternative_screen()      printf("\033[?1049h")
#define z_exit_alternative_screen()       printf("\033[?1049l")
#define z_clear_line()                    printf("\033[K")
#define z_clear_screen()                  printf("\033[2J")
#define z_update_screen()                 fflush(stdout)

bool z_enable_raw_mode(int vminKeys, int vtime);
bool z_disable_raw_mode();

bool z_get_cursor_pos(int *x, int *y);

bool z_get_screen_size_by_cursor(int *width, int *height);
bool z_get_screen_size_by_ioctl(int *width, int *height);
bool z_get_screen_size(int *width, int *height);

bool z_register_change_in_window_size(void function(int));

bool z_enable_full_buffering(FILE *fp);

int z_wait_for_byte();
int z_read_escape_key();
int z_read_key();

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   avl tree header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

#define Z_AVL_DECLARE(type_name, K, V, prefix)                         \
                                                                       \
typedef struct type_name {                                             \
    struct type_name *left;                                            \
    struct type_name *right;                                           \
    K key;                                                             \
    V value;                                                           \
    char height;                                                       \
} type_name;                                                           \
                                                                       \
void prefix##_put(type_name **root,                                    \
                     K key,                                            \
                     V value,                                          \
                     int cmp_keys(const K, const K),                               \
                     void free_key(K),                                 \
                     void free_value(V));                              \
                                                                       \
bool prefix##_is_exists(type_name *root,                               \
                        K key,                                         \
                        int cmp_keys(const K, const K));                           \
                                                                       \
bool prefix##_find(type_name *root,                                    \
                const K key,                                           \
                int cmp_keys(const K, const K),                                    \
                V *value);                                             \
                                                                       \
void prefix##_remove(type_name **root,                                 \
                     K key,                                            \
                     int cmp_keys(const K, const K),                               \
                     void free_key(K),                                 \
                     void free_value(V));                              \
                                                                       \
void prefix##_order_traverse(type_name *root,                          \
                             void action(K key, V value, void *arg),   \
                             void *arg);                               \
                                                                       \
void prefix##_print(type_name *root,                                   \
                    void print(K key, V value, void *arg),             \
                    void *arg, int padding);                           \
                                                                       \
void prefix##_free(type_name *root,                                    \
                   void free_key(K),                                   \
                   void free_value(V));



#define Z_AVL_IMPLEMENT(type_name, K, V, prefix)                              \
                                                                              \
int prefix##_get_height(const type_name *node)                                \
{                                                                             \
    if (node == NULL) {                                                       \
        return 0;                                                             \
    }                                                                         \
                                                                              \
    return node->height;                                                      \
}                                                                             \
                                                                              \
void prefix##_update_height(type_name *node)                                  \
{                                                                             \
    node->height = 1 + z_max(                                                 \
            prefix##_get_height(node->right),                                 \
            prefix##_get_height(node->left)                                   \
    );                                                                        \
}                                                                             \
                                                                              \
int prefix##_get_balance_factor(const type_name *node)                        \
{                                                                             \
    if (node == NULL) {                                                       \
        return 0;                                                             \
    }                                                                         \
                                                                              \
    return prefix##_get_height(node->left) - prefix##_get_height(node->right);\
}                                                                             \
                                                                              \
void prefix##_left_rotate(type_name **root)                                   \
{                                                                             \
    type_name *newRoot = (*root)->right;                                      \
    type_name *tmp = newRoot->left;                                           \
                                                                              \
    newRoot->left = *root;                                                    \
    (*root)->right = tmp;                                                     \
                                                                              \
    prefix##_update_height(newRoot->left);                                    \
    prefix##_update_height(newRoot);                                          \
                                                                              \
    *root = newRoot;                                                          \
}                                                                             \
                                                                              \
void prefix##_right_rotate(type_name **root)                                  \
{                                                                             \
    type_name *newRoot = (*root)->left;                                       \
    type_name *tmp = newRoot->right;                                          \
                                                                              \
    newRoot->right = *root;                                                   \
    (*root)->left = tmp;                                                      \
                                                                              \
    prefix##_update_height(newRoot->right);                                   \
    prefix##_update_height(newRoot);                                          \
                                                                              \
    *root = newRoot;                                                          \
}                                                                             \
                                                                              \
void prefix##_left_right_rotate(type_name **root)                             \
{                                                                             \
    prefix##_left_rotate(&(*root)->left);                                     \
    prefix##_right_rotate(root);                                              \
}                                                                             \
                                                                              \
void prefix##_right_left_rotate(type_name **root)                             \
{                                                                             \
    prefix##_right_rotate(&(*root)->right);                                   \
    prefix##_left_rotate(root);                                               \
}                                                                             \
                                                                              \
type_name *prefix##_new(K key, V value)                                       \
{                                                                             \
    type_name *n = malloc(sizeof(type_name));                                 \
    n->key = key;                                                             \
    n->value = value;                                                         \
    n->height = 1;                                                            \
    n->left = NULL;                                                           \
    n->right = NULL;                                                          \
                                                                              \
    return n;                                                                 \
}                                                                             \
                                                                              \
type_name *prefix##_get_min(type_name *root)                                  \
{                                                                             \
    type_name *curr = root;                                                   \
                                                                              \
    while (curr->left != NULL) {                                              \
        curr = curr->left;                                                    \
    }                                                                         \
                                                                              \
    return curr;                                                              \
}                                                                             \
                                                                              \
type_name *prefix##_find_node(type_name *root, const K key, int cmp_keys(const K, const K))     \
{                                                                             \
    type_name *curr = root;                                                   \
                                                                              \
    while (curr != NULL) {                                                    \
        int cmp_res = cmp_keys(key, curr->key);                               \
                                                                              \
        if (cmp_res > 0) {                                                    \
            curr = curr->right;                                               \
        } else if (cmp_res < 0) {                                             \
            curr = curr->left;                                                \
        } else {                                                              \
            return curr;                                                      \
        }                                                                     \
    }                                                                         \
                                                                              \
    return NULL;                                                              \
}                                                                             \
                                                                              \
bool prefix##_is_exists(type_name *root,                                      \
                        K key,                                                \
                        int cmp_keys(const K, const K))                                   \
{                                                                             \
    return prefix##_find_node(root, key, cmp_keys) != NULL;                        \
}                                                                             \
                                                                              \
bool prefix##_find(type_name *root,                                           \
                const K key,                                                  \
                int cmp_keys(const K, const K),                                           \
                V *value)                                                     \
{                                                                             \
    type_name *node = prefix##_find_node(root, key, cmp_keys);                     \
                                                                              \
    if (node != NULL) {                                                       \
        *value = node->value;                                                 \
        return true;                                                          \
    }                                                                         \
                                                                              \
    return false;                                                             \
}                                                                             \
                                                                              \
void prefix##_put(type_name **root,                                           \
                     K key,                                                   \
                     V value,                                                 \
                     int cmp_keys(const K, const K),                                      \
                     void free_key(K),                                        \
                     void free_value(V))                                      \
{                                                                             \
    if (*root == NULL) {                                                      \
        *root = prefix##_new(key, value);                                     \
        return;                                                               \
    }                                                                         \
                                                                              \
    int cmp_res = cmp_keys(key, (*root)->key);                                \
                                                                              \
    if (cmp_res > 0) {                                                        \
        prefix##_put(&(*root)->right, key, value, cmp_keys,                   \
                free_key, free_value);                                        \
    } else if (cmp_res < 0) {                                                 \
        prefix##_put(&(*root)->left, key, value, cmp_keys,                    \
                free_key, free_value);                                        \
    } else {                                                                  \
        CALL_F_IF_NOT_NULL(free_value, (*root)->value);                       \
        CALL_F_IF_NOT_NULL(free_key, key);                                  \
        (*root)->value = value;                                               \
    }                                                                         \
                                                                              \
    prefix##_update_height(*root);                                            \
                                                                              \
    int bf = prefix##_get_balance_factor(*root);                              \
                                                                              \
    if (bf > 1 && cmp_keys(key, (*root)->left->key) < 0) {                    \
        prefix##_right_rotate(root);                                          \
    } else if (bf < -1 && cmp_keys(key, (*root)->right->key) > 0) {           \
        prefix##_left_rotate(root);                                           \
    } else if (bf > 1 && cmp_keys(key, (*root)->left->key) > 0) {             \
        prefix##_left_right_rotate(root);                                     \
    } else if (bf < -1 && cmp_keys(key, (*root)->right->key) < 0) {           \
        prefix##_right_left_rotate(root);                                     \
    }                                                                         \
}                                                                             \
                                                                              \
void prefix##_remove(type_name **root,                                        \
                     K key,                                                   \
                     int cmp_keys(const K, const K),                                      \
                     void free_key(K),                                        \
                     void free_value(V))                                      \
{                                                                             \
    if (*root == NULL) {                                                      \
        return;                                                               \
    }                                                                         \
                                                                              \
    int cmp_res = cmp_keys(key, (*root)->key);                                \
                                                                              \
    if (cmp_res > 0) {                                                        \
        prefix##_remove(&((*root)->right), key, cmp_keys, free_key, free_value);   \
        return;                                                               \
    }                                                                         \
                                                                              \
    if (cmp_res < 0) {                                                        \
        prefix##_remove(&((*root)->left), key, cmp_keys, free_key, free_value);    \
        return;                                                               \
    }                                                                         \
                                                                              \
    /* we reached the node */                                                 \
                                                                              \
    CALL_F_IF_NOT_NULL(free_key, (*root)->key);                               \
    CALL_F_IF_NOT_NULL(free_value, (*root)->value);                           \
                                                                              \
    if ((*root)->left == NULL) {                                              \
        type_name *tmp = (*root)->right;                                      \
        free(*root);                                                          \
        *root = tmp;                                                          \
        return;                                                               \
    } else if ((*root)->right == NULL) {                                      \
        type_name *tmp = (*root)->left;                                       \
        free(*root);                                                          \
        *root = tmp;                                                          \
        return;                                                               \
    }                                                                         \
                                                                              \
    type_name *succesor = prefix##_get_min((*root)->right);                   \
                                                                              \
    (*root)->key = succesor->key;                                             \
    (*root)->value = succesor->value;                                         \
                                                                              \
    prefix##_remove(&((*root)->right), succesor->key, cmp_keys, NULL, NULL);  \
                                                                              \
    prefix##_update_height(*root);                                            \
    int bf = prefix##_get_balance_factor(*root);                              \
                                                                              \
    if (bf > 1 && prefix##_get_balance_factor((*root)->left) >= 0) {          \
        prefix##_right_rotate(root);                                          \
    } else if (bf < -1 && prefix##_get_balance_factor((*root)->right) <= 0) { \
        prefix##_left_rotate(root);                                           \
    } else if (bf > 1 && prefix##_get_balance_factor((*root)->left) < 0) {    \
        prefix##_left_right_rotate(root);                                     \
    } else if (bf < -1 && prefix##_get_balance_factor((*root)->right) > 0) {  \
        prefix##_right_left_rotate(root);                                     \
    }                                                                         \
}                                                                             \
                                                                              \
void prefix##_order_traverse(type_name *root,                                 \
                             void action(K key, V value, void *arg),          \
                             void *arg)                                       \
{                                                                             \
    if (root == NULL) {                                                       \
        return;                                                               \
    }                                                                         \
                                                                              \
    prefix##_order_traverse(root->left, action, arg);                              \
                                                                              \
    action(root->key, root->value, arg);                                      \
    prefix##_order_traverse(root->right, action, arg);                             \
}                                                                             \
                                                                              \
void print_char_n_times(char c, int n)                                        \
{                                                                             \
    for (int i = 0; i < n; i++) {                                             \
        putchar(c);                                                           \
    }                                                                         \
}                                                                             \
                                                                              \
void prefix##_print(type_name *root,                                          \
                    void print(K key, V value, void *arg),                    \
                    void *arg, int padding)                                   \
{                                                                             \
    if (root == NULL) {                                                       \
        return;                                                               \
    }                                                                         \
                                                                              \
    print_char_n_times(' ', padding);                                         \
    print(root->key, root->value, arg);                                       \
    prefix##_print(root->right, print, arg, padding + 4);                          \
    prefix##_print(root->left, print, arg, padding + 4);                           \
}                                                                             \
                                                                              \
void prefix##_free(type_name *root,                                           \
                   void free_key(K),                                          \
                   void free_value(V))                                        \
{                                                                             \
    if (root == NULL) {                                                       \
        return;                                                               \
    }                                                                         \
                                                                              \
    CALL_F_IF_NOT_NULL(free_key, root->key);                                  \
    CALL_F_IF_NOT_NULL(free_value, root->value);                              \
                                                                              \
    prefix##_free(root->left, free_key, free_value);                               \
    prefix##_free(root->right, free_key, free_value);                              \
                                                                              \
    free(root);                                                               \
}

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   map header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *


#define Z_MAP_DECLARE(type_name, K, V, prefix)                         \
                                                                       \
Z_AVL_DECLARE(_avl_##type_name, K, V, _avl_##prefix)                   \
                                                                       \
typedef struct {                                                       \
    _avl_##type_name *root;                                            \
    int (*cmp_keys)(const K, const K);                                           \
} type_name;                                                           \
                                                                       \
void prefix##_init(type_name *m, int cmp_keys(const K, const K));                  \
                                                                       \
void prefix##_put(type_name *m,                                        \
                  K key,                                               \
                  V value,                                             \
                  void free_key(K),                                    \
                  void free_value(V));                                 \
                                                                       \
bool prefix##_find(const type_name *m, const K key, V *value);         \
                                                                       \
bool prefix##_is_exists(const type_name *m, K key);                    \
                                                                       \
void prefix##_remove(type_name *m, K key,                              \
                     void free_key(K),                                 \
                     void free_value(V));                              \
                                                                       \
void prefix##_order_traverse(const type_name *m,                       \
                             void action(K key, V value, void *arg),   \
                             void *arg);                               \
                                                                       \
void prefix##_free(type_name *m,                                       \
                   void free_key(K),                                   \
                   void free_value(V));

#define Z_MAP_IMPLEMENT(type_name, K, V, prefix)                       \
                                                                       \
Z_AVL_IMPLEMENT(_avl_##type_name, K, V, _avl_##prefix)                 \
                                                                       \
void prefix##_init(type_name *m, int cmp_keys(const K, const K))                   \
{                                                                      \
    m->cmp_keys = cmp_keys;                                            \
    m->root = NULL;                                                    \
}                                                                      \
                                                                       \
void prefix##_put(type_name *m,                                        \
                  K key,                                               \
                  V value,                                             \
                  void free_key(K),                                    \
                  void free_value(V))                                  \
{                                                                      \
    _avl_##prefix##_put(&m->root, key, value, m->cmp_keys, free_key, free_value);\
}                                                                      \
                                                                       \
bool prefix##_find(const type_name *m, const K key, V *value)          \
{                                                                      \
    return _avl_##prefix##_find(m->root, key, m->cmp_keys, value);     \
}                                                                      \
                                                                       \
bool prefix##_is_exists(const type_name *m, K key)                     \
{                                                                      \
    return _avl_##prefix##_is_exists(m->root, key, m->cmp_keys);       \
}                                                                      \
                                                                       \
void prefix##_remove(type_name *m, K key,                              \
                     void free_key(K),                                 \
                     void free_value(V))                               \
{                                                                      \
    _avl_##prefix##_remove(&m->root,                                   \
            key,                                                       \
            m->cmp_keys,                                               \
            free_key,                                                  \
            free_value);                                               \
}                                                                      \
                                                                       \
void prefix##_order_traverse(const type_name *m,                       \
                             void action(K key, V value, void *arg),   \
                             void *arg)                                \
{                                                                      \
    _avl_##prefix##_order_traverse(m->root, action, arg);              \
}                                                                      \
                                                                       \
void prefix##_free(type_name *m,                                       \
                   void free_key(K),                                   \
                   void free_value(V))                                 \
{                                                                      \
    _avl_##prefix##_free(m->root, free_key, free_value);               \
}


//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   matrix header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

#define Z_MAT_DECLARE(name, T)    \
typedef struct {                  \
    T *ptr;                       \
    int x;                        \
    int y;                        \
    int capacity;                 \
} name;

#define Z_MAT_AT(mat, _y, _x) ((mat)->ptr[(_y) * (mat)->x + (_x)])

#define Z_MAT_INIT(mat, _y, _x)                                     \
    do {                                                            \
        (mat)->ptr = malloc(sizeof((mat)->ptr[0]) * (_x) * (_y));   \
        (mat)->x = (_x);                                            \
        (mat)->y = (_y);                                            \
        (mat)->capacity = (_x) * (_y);                              \
    } while (0)

#define Z_MAT_RESIZE(mat, _y, _x)                                                   \
    do {                                                                            \
        if ((mat)->capacity < (_x) * (_y)) {                                        \
            (mat)->capacity = (_x) * (_y);                                          \
            (mat)->ptr = realloc((mat)->ptr, sizeof((mat)->ptr[0]) * (_x) * (_y));  \
        }                                                                           \
                                                                                    \
        (mat)->x = (_x);                                                            \
        (mat)->y = (_y);                                                            \
    } while (0)

#define Z_MAT_FREE(mat) do { free((mat)->ptr); (mat)->ptr = NULL; } while (0)

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   string header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

typedef struct {
    char *ptr;
    int len;
    int capacity;
} Z_String;

typedef struct {
    const char *ptr;
    int len;
} Z_String_View;

typedef struct {
    const char *curr;
    const char *end;
    Z_String_View delim;
} Z_String_Tokonizer;

#define Z_SV(p, l)          ((Z_String_View){ .ptr = (p), .len = (l)            })
#define Z_STR_TO_SV(s)      ((Z_String_View){ .ptr = (s).ptr, .len = (s).len    })
#define Z_CSTR_TO_SV(s)     ((Z_String_View){ .ptr = (s), .len = strlen(s)      })
#define Z_EMPTY_SV()        ((Z_String_View){ .ptr = "", .len = 0               })

const char *z_str_to_cstr(Z_String *s);
Z_String z_str_new(const char *fmt, ...);
Z_String z_str_new_va(const char *fmt, va_list ap);
void z_str_pushf(Z_String *s, const char *fmt, ...);
void z_str_pushf_va(Z_String *s, const char *fmt, va_list ap);
void z_str_push(Z_String *dst, Z_String_View src);
void z_str_push_c(Z_String *s, char c);
char z_str_pop_c(Z_String *s);
char z_str_top_c(Z_String_View s);
int z_str_cmp(Z_String_View s1, Z_String_View s2);
int z_str_n_cmp(Z_String_View s1, Z_String_View s2, int n);
void z_str_replace(Z_String *s, Z_String_View target, Z_String_View replacement);
char *z_sv_to_cstr(Z_String_View s);

void z_str_tok_init(Z_String_Tokonizer *tok, Z_String_View s, Z_String_View delim);
Z_String_View z_str_tok_next(Z_String_Tokonizer *tok);

void z_str_trim(Z_String *s);
void z_str_trim_cset(Z_String *s, Z_String_View cset);
Z_String_View z_str_view_trim(Z_String_View s);
Z_String_View z_str_view_trim_cset(Z_String_View s, Z_String_View cset);

void z_str_print(Z_String_View s);
void z_str_println(Z_String_View s);
void z_str_free(Z_String *s);
void z_str_clear(Z_String *s);

bool z_read_whole_file(const char *pathname, Z_String *out);
void z_str_get_line(FILE *fp, Z_String *out);

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   path header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

typedef enum {
    Z_Pipe_Mode_Read = 0,
    Z_Pipe_Mode_Write = 1,
} Z_Pipe_Mode;

Z_String_View z_get_path_extention(Z_String_View path);
Z_String_View z_get_path_basename(Z_String_View path);

Z_String_View z_get_home_path();

void z_expand_path(Z_String_View p, Z_String *out);
Z_String z_compress_path(Z_String_View s);

bool z_is_extention_equal(Z_String_View pathname, Z_String_View extention);

bool z_dir_traverse(const char *dir, bool action(const char *));

bool z_is_dir(const char *pathname);
bool z_is_regular_file(const char *pathname);
bool z_is_path_exists(const char *pathname);

bool z_write_file(const char *pathname, const char *fmt, ...);
bool z_append_file(const char *pathname, const char *fmt, ...);
bool z_read_file(const char *pathname, const char *fmt, ...);

bool z_redirect_fd(int src_fd, const char *dst_pathname);
bool z_popen2(char *path, char *argv[], FILE *ppipe[2]);

bool z_mkdir(const char *pathname);


//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   cmd header
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

typedef struct {
	char **ptr;
	int len;
    int capacity;
} Z_Cmd;

bool _z_should_rebuild(const char *target, ...);
bool z_should_rebuild_va(const char *target, va_list ap);
#define z_should_rebuild(target, ...) _z_should_rebuild(target, ##__VA_ARGS__, NULL)
void z_rebuild_yourself(const char *src_pathname, char **argv);
void z_cmd_init(Z_Cmd *cmd);
#define z_cmd_append(cmd, ...) _z_cmd_append(cmd, __VA_ARGS__, NULL)
void _z_cmd_append(Z_Cmd *cmd, ...);
void z_cmd_append_va(Z_Cmd *cmd, va_list ap);
int z_cmd_run_async(Z_Cmd *cmd);
int _z_run_async(const char *arg, ...);
#define z_run_async(arg, ...) _z_run_async(arg, ##__VA_ARGS__, NULL)
void z_cmd_free(Z_Cmd *cmd);
void z_cmd_clear(Z_Cmd *cmd);

//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $


#ifdef LIBZATAR_IMPLEMENTATION


//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $




//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   util implementation
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

int z_print_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("[" Z_COLOR_RED "ERROR" Z_COLOR_RESET "] ");
    int n = vprintf(fmt, ap);
    printf("\n");
    va_end(ap);

    return n;
}

int z_print_warning(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("[" Z_COLOR_YELLOW "WARNING" Z_COLOR_RESET "] ");
    int n = vprintf(fmt, ap);
    printf("\n");
    va_end(ap);

    return n;
}

int z_print_info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("[" Z_COLOR_GREEN "INFO" Z_COLOR_RESET "] ");
    int n = vprintf(fmt, ap);
    printf("\n");
    va_end(ap);

    return n;
}

void z_swap(void *a, void *b, const size_t size)
{
    char tmp[size];
    memcpy(tmp, a, size);
    memcpy(a, b, size);
    memcpy(b, tmp, size);
}

int z_get_file_size(FILE *fp)
{
    int curr = ftell(fp);
    fseek(fp, 0, SEEK_END);

    int size = ftell(fp);
    fseek(fp, curr, SEEK_SET);

    return size;
}

int z_get_fmt_size(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int size = z_get_fmt_size_va(fmt, ap);
    va_end(ap);

    return size;
}

int z_get_fmt_size_va(const char *fmt, va_list ap)
{
    va_list ap1;
    va_copy(ap1, ap);

    int size = vsnprintf(NULL, 0, fmt, ap1);
    va_end(ap1);

    return size;
}

void *z_memdup(const void *mem, const size_t size)
{
    void *newMem = malloc(size);
    memcpy(newMem, mem, size);
    return newMem;
}

int z_max(int a, int b)
{
    return a > b ? a : b;
}

int z_min(int a, int b)
{
    return a > b ? b : a;
}

int z_min3(int a, int b, int c)
{
    return z_min(a, z_min(b, c));
}

int z_max3(int a, int b, int c)
{
    return z_max(a, z_max(b, c));
}

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   cursor implementation
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

static struct termios original_termios;

bool z_enable_raw_mode(int vminKeys, int vtime)
{
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        return false;
    }

    struct termios raw = original_termios;
    raw.c_cc[VMIN] = vminKeys;
    raw.c_cc[VTIME] = vtime;
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST);
    raw.c_iflag &= ~(IXON | ICRNL | ISTRIP | INPCK | BRKINT);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return false;
    }

    return true;
}

bool z_disable_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) {
        return false;
    }

    return true;
}

bool z_get_cursor_pos(int *x, int *y)
{
    printf("\033[6n");

    if (scanf("\033[%d;%dR", y, x) == 2) {
        return true;
    }

    return false;
}

bool z_get_screen_size_by_cursor(int *width, int *height)
{
    z_set_cursor_pos(999, 999);

    return z_get_cursor_pos(width, height);
}

bool z_get_screen_size_by_ioctl(int *width, int *height)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        return false;
    }

    *width = ws.ws_col;
    *height = ws.ws_row;

    return true;
}

bool z_get_screen_size(int *width, int *height)
{
    if (z_get_screen_size_by_ioctl(width, height) == true) {
        return true;
    }

    return z_get_screen_size_by_cursor(width, height);
}

bool z_register_change_in_window_size(void funciton(int))
{
    struct sigaction sa;
    sa.sa_handler = funciton;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        return false;
    }

    return true;
}

bool z_enable_full_buffering(FILE *fp)
{
    // do not flush on '\n'
    if (setvbuf(fp, NULL, _IOFBF, BUFSIZ) != 0) {
        return false;
    }

    return true;
}

int z_wait_for_byte()
{
    char c;

    if (read(STDIN_FILENO, &c, 1) != 1) {
        return Z_KEY_EMPTY;
    }

    return c;
}

#define CHAR2_TO_INT(a, b) ((unsigned short)((((unsigned short)b) << 8) | a))

int z_read_escape_key()
{
    unsigned short key;

    if (read(STDIN_FILENO, &key, 2) != 2) {
        return '\033';
    }

    switch (key) {
        case CHAR2_TO_INT('[', 'A'): return Z_KEY_ARROW_UP;
        case CHAR2_TO_INT('[', 'B'): return Z_KEY_ARROW_DOWN;
        case CHAR2_TO_INT('[', 'C'): return Z_KEY_ARROW_RIGHT;
        case CHAR2_TO_INT('[', 'D'): return Z_KEY_ARROW_LEFT;
        case CHAR2_TO_INT('[', '1'): return Z_KEY_HOME; // might be with a ~
        case CHAR2_TO_INT('[', '5'): return Z_KEY_PAGE_UP; // might be with a ~
        case CHAR2_TO_INT('[', '6'): return Z_KEY_PAGE_DOWN; // might be with a ~
    }

    return '\033';
}

int z_read_key()
{
    char c = z_wait_for_byte();

    if (c == '\033') {
        return z_read_escape_key();
    }

    return c;
}

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   path implementation
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

Z_String_View z_get_path_extention(Z_String_View path)
{
    int start = path.len - 1;

    while (start > 0 && path.ptr[start] != '.') {
        if (path.ptr[start] == '/') {
            return Z_EMPTY_SV();
        }

        start--;
    }

    if (start == 0) {
        if (path.ptr[0] == '.') {
            return Z_SV(path.ptr + 1, path.len - 1);
        } else {
            return Z_EMPTY_SV();
        }
    }

    Z_String_View extention = {
        .ptr = path.ptr + start + 1,
        .len = path.len - start - 1,
    };

    return extention;
}

Z_String_View z_get_path_basename(Z_String_View pathname)
{
    (void)pathname;
    return Z_EMPTY_SV();
}

Z_String_View z_get_home_path()
{
    const char *home = getenv("HOME");

    if (home == NULL) {
        return Z_CSTR_TO_SV(".");
    }

    return Z_CSTR_TO_SV(home);
}

void z_expand_path(Z_String_View p, Z_String *out)
{
    if (p.len == 0) {
        return;
    }

    Z_String_View home = z_get_home_path();

    if (p.ptr[0] == '~') {
        z_str_pushf(out, "%.*s%.*s", home.len, home.ptr, p.len - 1, p.ptr + 1);
        return;
    }

    z_str_pushf(out, "%.*s", p.len, p.ptr);
    return;
}

Z_String z_compress_path(Z_String_View p)
{
    Z_String_View home = z_get_home_path();

    if (home.len <= p.len && z_str_n_cmp(p, home, home.len) == 0) {
        return z_str_new("~%.*s", p.len - home.len, p.ptr + home.len);
    }

    return z_str_new("%.*s", p.len, p.ptr);
}

bool z_dir_traverse(const char *dir, bool action(const char *))
{
    DIR *dr = opendir(dir);

    if (dr == NULL) {
        return false;
    }

    struct dirent *de;

    Z_String full_path = {0};

    while ((de = readdir(dr))) {

        z_str_clear(&full_path);
        z_str_pushf(&full_path, "%s/%s", dir, de->d_name);

        if (action(full_path.ptr) == false) {
            break;
        }
    }

    z_str_free(&full_path);
    closedir(dr);

    return true;
}

bool z_is_extention_equal(Z_String_View pathname, Z_String_View extention)
{
    return z_str_cmp(z_get_path_extention(pathname), extention) == 0;
}

bool z_is_dir(const char *pathname)
{
    struct stat sb;
    stat(pathname, &sb);

    return S_ISDIR(sb.st_mode);
}

bool z_is_regular_file(const char *pathname)
{
    struct stat sb;
    stat(pathname, &sb);

    return S_ISREG(sb.st_mode);
}

bool z_is_path_exists(const char *pathname)
{
    return !access(pathname, F_OK);
}

bool z_write_file(const char *fileName, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    FILE *fp = fopen(fileName, "w");

    if (fp == NULL) {
        return false;
    }

    vfprintf(fp, fmt, ap);
    va_end(ap);

    return true;
}

bool z_append_file(const char *fileName, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    FILE *fp = fopen(fileName, "a");

    if (fp == NULL) {
        return false;
    }

    vfprintf(fp, fmt, ap);
    va_end(ap);

    return true;
}

bool z_read_file(const char *fileName, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    FILE *fp = fopen(fileName, "r");

    if (fp == NULL) {
        return false;
    }

    if (vfscanf(fp, fmt, ap) == EOF) {
        fclose(fp);
        return false;
    }

    va_end(ap);
    fclose(fp);

    return true;
}

bool z_redirect_fd(int srcFd, const char *destFileName)
{
    int destFd = open(destFileName, O_WRONLY);

    if (destFd == -1) {
        return false;
    }

    if (dup2(destFd, srcFd) == -1) {
        close(destFd);
        return false;
    }

    close(destFd);

    return true;
}

bool z_popen2(char *pathname, char *argv[], FILE *ppipe[2])
{
    int output[2];
    int input[2];

    if (pipe(output) == -1 || pipe(input) == -1) {
        return false;
    }

    int pid = fork();

    if (pid == -1) {
        return false;
    }

    if (pid) {
        // parent
        close(output[Z_Pipe_Mode_Write]);
        ppipe[Z_Pipe_Mode_Write] = fdopen(input[Z_Pipe_Mode_Write], "w");
        ppipe[Z_Pipe_Mode_Read] = fdopen(output[Z_Pipe_Mode_Read], "r");
    } else {
        // child
        dup2(input[Z_Pipe_Mode_Read], STDIN_FILENO);
        dup2(output[Z_Pipe_Mode_Write], STDOUT_FILENO);
        close(input[Z_Pipe_Mode_Write]);
        close(input[Z_Pipe_Mode_Read]);
        close(output[Z_Pipe_Mode_Write]);
        close(output[Z_Pipe_Mode_Read]);
        execvp(pathname, argv);
        exit(EXIT_FAILURE);
    }

    return true;
}

bool z_mkdir(const char *pathname)
{
    int status = mkdir(pathname, 0777);

    if (status == 0) {
        return true;
    } else if (errno == EEXIST) {
        z_print_warning("cannot create directory ‘%s’: File exists", pathname);
    } else {
        z_print_error("%s", strerror(errno));
    }

    return false;
}

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   string implementation
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

const char *z_str_to_cstr(Z_String *s)
{
    z_da_null_terminate(s);
    return s->ptr;
}

Z_String z_str_new(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    Z_String s = z_str_new_va(fmt, ap);
    va_end(ap);

    return s;
}

Z_String z_str_new_va(const char *fmt, va_list ap)
{
    Z_String s = {0};
    z_str_pushf_va(&s, fmt, ap);

    return s;
}

void z_str_pushf(Z_String *s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    z_str_pushf_va(s, fmt, ap);
    va_end(ap);
}

void z_str_pushf_va(Z_String *s, const char *fmt, va_list ap)
{
    int len = z_get_fmt_size_va(fmt, ap);
    z_da_ensure_capacity(s, s->len + len + 1);

    va_list ap1;
    va_copy(ap1, ap);
    vsnprintf(s->ptr + s->len, len + 1, fmt, ap1);
    va_end(ap1);

    s->len = s->len + len;
}

void z_str_push(Z_String *dst, Z_String_View src)
{
    z_str_pushf(dst, "%.*s", src.len, src.ptr);
}

void z_str_push_c(Z_String *s, char c)
{
    z_da_ensure_capacity(s, s->len + 1);
    s->ptr[s->len++] = c;
    z_da_null_terminate(s);
}

char z_str_pop_c(Z_String *s)
{
    return s->ptr[--s->len];
}

char z_str_top_c(Z_String_View s)
{
    return s.ptr[s.len - 1];
}

int z_str_cmp(Z_String_View s1, Z_String_View s2)
{
    if (s1.len > s2.len) {
        return 1;
    } else if (s1.len < s2.len) {
        return -1;
    } else {
        return memcmp(s1.ptr, s2.ptr, s1.len);
    }
}

int z_str_n_cmp(Z_String_View s1, Z_String_View s2, int n)
{
    assert(s1.len >= n && s2.len >= n);

    return memcmp(s1.ptr, s2.ptr, n);
}

void z_str_replace(Z_String *s, Z_String_View target, Z_String_View replacement);

char *z_sv_to_cstr(Z_String_View s)
{
    return strndup(s.ptr, s.len);
}

void z_str_tok_init(Z_String_Tokonizer *tok, Z_String_View s, Z_String_View delim)
{
    tok->curr = s.ptr;
    tok->end = s.ptr + s.len;
    tok->delim = delim;
}

int z_str_chr(Z_String_View s, char c)
{
    for (int i = 0; i < s.len; i++) {
        if (s.ptr[i] == c) {
            return i;
        }
    }

    return -1;
}

Z_String_View z_str_tok_next(Z_String_Tokonizer *tok)
{
    while (tok->curr < tok->end && z_str_chr(tok->delim, *tok->curr) >= 0) {
        tok->curr++;
    }

    if (tok->curr >= tok->end) {
        return Z_EMPTY_SV();
    }

    const char *start = tok->curr;

    while (tok->curr < tok->end && z_str_chr(tok->delim, *tok->curr) < 0) {
        tok->curr++;
    }

    return Z_SV(start, tok->curr - start);
}

void z_str_trim(Z_String *s)
{
    z_str_trim_cset(s, Z_CSTR_TO_SV(" \f\t\v\n\r"));
}

void z_str_trim_cset(Z_String *s, Z_String_View cset)
{
    Z_String_View trimmed = z_str_view_trim_cset(Z_STR_TO_SV(*s), cset);
    memmove(s->ptr, trimmed.ptr, trimmed.len);
    s->len = trimmed.len;
}

Z_String_View z_str_view_trim(Z_String_View s)
{
    return z_str_view_trim_cset(s, Z_CSTR_TO_SV(" \f\t\v\n\r"));
}

Z_String_View z_str_view_trim_cset(Z_String_View s, Z_String_View cset)
{
    const char *start = s.ptr;
    const char *end = s.ptr + s.len - 1;

    while (start < end && z_str_chr(cset, *start) >= 0) {
        start++;
    }

    while (start < end && z_str_chr(cset, *end) >= 0) {
        end--;
    }

    Z_String_View ret = {
        .ptr = start,
        .len = end - start + 1,
    };

    return ret;
}

void z_str_print(Z_String_View s)
{
    printf("%.*s", s.len, s.ptr);
}

void z_str_println(Z_String_View s)
{
    printf("%.*s\n", s.len, s.ptr);
}

void z_str_free(Z_String *s)
{
    free(s->ptr);
    s->ptr = NULL;
    s->len = 0;
    s->capacity = 0;
}

void z_str_clear(Z_String *s)
{
    s->len = 0;
    z_da_null_terminate(s);
}

bool z_read_whole_file(const char *pathname, Z_String *out)
{
    FILE *fp = fopen(pathname, "r");

    if (fp == NULL) {
        return false;
    }

    int file_size = z_get_file_size(fp);

    z_da_ensure_capacity(out, out->len + file_size);
    out->len += fread(&out->ptr[out->len], sizeof(char), file_size, fp);
    z_da_null_terminate(out);

    fclose(fp);
    return true;
}

void z_str_get_line(FILE *fp, Z_String *out)
{
    char buf[BUFSIZ];

    while (fgets(buf, BUFSIZ, fp)) {
        z_str_pushf(out, "%s", buf);
    }
}

//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//
//
//   cmd implementation
//
//
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *
//       *       *       *       *       *       *        *        *
//   *       *       *       *       *       *       *        *        *

bool _z_should_rebuild(const char *target, ...)
{
    va_list ap;
    va_start(ap, target);
    bool should_rebuild = z_should_rebuild_va(target, ap);
    va_end(ap);

    return should_rebuild;
}

bool z_should_rebuild_va(const char *target, va_list ap)
{
    va_list ap1;
    va_copy(ap1, ap);

	struct stat target_stat;
	struct stat dependency_stat;

    if (stat(target, &target_stat)) {
        return true;
    }

    const char *dependency = va_arg(ap, const char *);

    while (dependency) {
        if (stat(dependency, &dependency_stat)) {
            z_print_error("cannot access '%s': No such file", dependency);
            va_end(ap1);
            return false;
        } else if (target_stat.st_mtim.tv_sec < dependency_stat.st_mtim.tv_sec) {
            va_end(ap1);
            return true;
        }

        dependency = va_arg(ap, const char *);
    }

    return false;
}

void z_rebuild_yourself(const char *src_pathname, char **argv)
{
    if (!z_should_rebuild(argv[0], src_pathname, __FILE__)) {
        return;
    }

    Z_Cmd cmd = {0};
    z_cmd_append(&cmd, "cc", src_pathname, "-o", argv[0]);
    int status = z_cmd_run_async(&cmd);
    z_cmd_free(&cmd);

    if (status != 0) {
        exit(status);
    }

	status = execvp(argv[0], argv);
    exit(1);
}

void z_cmd_init(Z_Cmd *cmd)
{
    cmd->ptr = NULL;
    cmd->len = 0;
    cmd->capacity = 0;
}

void _z_cmd_append(Z_Cmd *cmd, ...)
{
	va_list ap;
    va_start(ap, cmd);
    z_cmd_append_va(cmd, ap);
	va_end(ap);
}

void z_cmd_append_va(Z_Cmd *cmd, va_list ap)
{
	va_list ap1;
    va_copy(ap1, ap);

	const char *arg = va_arg(ap1, const char *);

	while (arg) {
        z_da_ensure_capacity(cmd, cmd->len + 1);
		cmd->ptr[cmd->len++] = strdup(arg);
		arg = va_arg(ap1, const char *);
	}

	va_end(ap1);
}

void z_cmd_print_arg(const char *arg)
{
	if (strchr(arg, ' ')) {
		printf("'%s'", arg);
	} else {
		printf("%s", arg);
	}
}

void z_cmd_print(const Z_Cmd *cmd)
{
	printf("[" Z_COLOR_GREEN "CMD" Z_COLOR_RESET "]");

	for (int i = 0; i < cmd->len; i++) {
		printf(" ");
		z_cmd_print_arg(cmd->ptr[i]);
	}

	printf("\n");
}

int z_cmd_run_async(Z_Cmd *cmd)
{
    z_da_null_terminate(cmd);
	z_cmd_print(cmd);

	pid_t pid = fork();
	int status = 0;

	if (pid == -1) {
		z_print_error("fork couln't create child");
        return -1;
	} else if (pid == 0) {
		exit(execvp(cmd->ptr[0], cmd->ptr));
	} else {
		waitpid(pid, &status, 0);
	}

	if (status != 0) {
		z_print_error(Z_COLOR_RED "exited abnormally " Z_COLOR_RESET
                "with code " Z_COLOR_RED "%d" Z_COLOR_RESET, status);
	}

	return status;
}

void z_cmd_free(Z_Cmd *cmd)
{
    for (int i = 0; i < cmd->len; i++) {
        free(cmd->ptr[i]);
    }

    free(cmd->ptr);
}

void z_cmd_clear(Z_Cmd *cmd)
{
    for (int i = 0; i < cmd->len; i++) {
        free(cmd->ptr[i]);
    }

    cmd->len = 0;
}
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $

#endif // end implementation
#endif // end header

#ifndef LIBZATAR_STRIP_PREFIX_GAURD
#define LIBZATAR_KEEP_PREFIX_GAURD

#ifdef LIBZATAR_STRIP_PREFIX

#define DEFAULT_GROWTH_RATE Z_DEFAULT_GROWTH_RATE
#define in_range z_in_range
#define get_file_size z_get_file_size
#define get_fmt_size z_get_fmt_size
#define get_fmt_size_va z_get_fmt_size_va
#define memdup z_memdup
#define swap z_swap
#define max z_max
#define min z_min
#define max3 z_max3
#define min3 z_min3
#define print_error z_print_error
#define print_info z_print_info
#define print_warning z_print_warning
#define ensure_capacity z_da_ensure_capacity
#define null_terminate z_da_null_terminate
#define COLOR_RESET Z_COLOR_RESET
#define COLOR_RED Z_COLOR_RED
#define COLOR_GREEN Z_COLOR_GREEN
#define COLOR_YELLOW Z_COLOR_YELLOW
#define COLOR_BLUE Z_COLOR_BLUE
#define COLOR_MAGENTA Z_COLOR_MAGENTA
#define COLOR_CYAN Z_COLOR_CYAN
#define COLOR_WHITE Z_COLOR_WHITE
#define COLOR_GRAY Z_COLOR_GRAY
#define COLOR_BOLD_RED Z_COLOR_BOLD_RED
#define COLOR_BOLD_GREEN Z_COLOR_BOLD_GREEN
#define COLOR_BOLD_YELLOW Z_COLOR_BOLD_YELLOW
#define COLOR_BOLD_BLUE Z_COLOR_BOLD_BLUE
#define COLOR_BOLD_MAGENTA Z_COLOR_BOLD_MAGENTA
#define COLOR_BOLD_CYAN Z_COLOR_BOLD_CYAN
#define COLOR_BOLD_WHITE Z_COLOR_BOLD_WHITE
#define COLOR_BOLD_GRAY Z_COLOR_BOLD_GRAY
#define KEY_EMPTY Z_KEY_EMPTY
#define KEY_ARROW_LEFT Z_KEY_ARROW_LEFT
#define KEY_ARROW_RIGHT Z_KEY_ARROW_RIGHT
#define KEY_ARROW_UP Z_KEY_ARROW_UP
#define KEY_ARROW_DOWN Z_KEY_ARROW_DOWN
#define KEY_PAGE_UP Z_KEY_PAGE_UP
#define KEY_PAGE_DOWN Z_KEY_PAGE_DOWN
#define KEY_DELETE Z_KEY_DELETE
#define KEY_HOME Z_KEY_HOME
#define KEY_END Z_KEY_END
#define CURSOR_STYLE Z_CURSOR_STYLE
#define CURSOR_STYLE_BLOCK_STEADY Z_CURSOR_STYLE_BLOCK_STEADY
#define CURSOR_STYLE_BLOCK_BLINKING Z_CURSOR_STYLE_BLOCK_BLINKING
#define CURSOR_STYLE_UNDERLINE_BLINKING Z_CURSOR_STYLE_UNDERLINE_BLINKING
#define CURSOR_STYLE_UNDERLINE_STEADY Z_CURSOR_STYLE_UNDERLINE_STEADY
#define CURSOR_STYLE_BEAM_STEADY Z_CURSOR_STYLE_BEAM_STEADY
#define CURSOR_STYLE_BEAM_BLINKING Z_CURSOR_STYLE_BEAM_BLINKING
#define set_cursor_style z_set_cursor_style
#define disable_line_wrap z_disable_line_wrap
#define enbale_line_wrap z_enbale_line_wrap
#define hide_cursor z_hide_cursor
#define show_cursor z_show_cursor
#define set_cursor_pos z_set_cursor_pos
#define set_cursor_x z_set_cursor_x
#define cursor_up z_cursor_up
#define cursor_down z_cursor_down
#define cursor_right z_cursor_right
#define cursor_left z_cursor_left
#define save_cursor_pos z_save_cursor_pos
#define restore_cursor_pos z_restore_cursor_pos
#define enter_alternative_screen z_enter_alternative_screen
#define exit_alternative_screen z_exit_alternative_screen
#define clear_line z_clear_line
#define clear_screen z_clear_screen
#define update_screen z_update_screen
#define enable_raw_mode z_enable_raw_mode
#define disable_raw_mode z_disable_raw_mode
#define get_cursor_pos z_get_cursor_pos
#define get_screen_size_by_cursor z_get_screen_size_by_cursor
#define get_screen_size_by_ioctl z_get_screen_size_by_ioctl
#define get_screen_size z_get_screen_size
#define register_change_in_window_size z_register_change_in_window_size
#define enable_full_buffering z_enable_full_buffering
#define wait_for_byte z_wait_for_byte
#define read_escape_key z_read_escape_key
#define read_key z_read_key
#define VECTOR_DECLARE Z_VECTOR_DECLARE
#define VECTOR_IMPLEMENT Z_VECTOR_IMPLEMENT
#define AVL_DECLARE Z_AVL_DECLARE
#define AVL_IMPLEMENT Z_AVL_IMPLEMENT
#define MAP_DECLARE Z_MAP_DECLARE
#define MAP_IMPLEMENT Z_MAP_IMPLEMENT
#define MAT_DECLARE Z_MAT_DECLARE
#define MAT_AT Z_MAT_AT
#define MAT_INIT Z_MAT_INIT
#define MAT_RESIZE Z_MAT_RESIZE
#define MAT_FREE Z_MAT_FREE
#define String Z_String
#define String_View Z_String_View
#define String_Tokonizer Z_String_Tokonizer
#define SV Z_SV
#define STR_TO_SV Z_STR_TO_SV
#define CSTR_TO_SV Z_CSTR_TO_SV
#define EMPTY_SV Z_EMPTY_SV
#define str_new z_str_new
#define str_new_va z_str_new_va
#define str_pushf z_str_pushf
#define str_pushf_va z_str_pushf_va
#define str_push z_str_push
#define str_push_c z_str_push_c
#define str_pop_c z_str_pop_c
#define str_top_c z_str_top_c
#define str_cmp z_str_cmp
#define str_n_cmp z_str_n_cmp
#define str_replace z_str_replace
#define sv_to_cstr z_sv_to_cstr
#define str_tok_init z_str_tok_init
#define str_tok_next z_str_tok_next
#define str_trim z_str_trim
#define str_trim_cset z_str_trim_cset
#define str_view_trim z_str_view_trim
#define str_view_trim_cset z_str_view_trim_cset
#define str_print z_str_print
#define str_println z_str_println
#define str_free z_str_free
#define str_clear z_str_clear
#define read_whole_file z_read_whole_file
#define Pipe_Mode Z_Pipe_Mode
#define Pipe_Mode_Read Z_Pipe_Mode_Read
#define Pipe_Mode_Write Z_Pipe_Mode_Write
#define get_path_extention z_get_path_extention
#define get_path_basename z_get_path_basename
#define get_home_path z_get_home_path
#define expand_path z_expand_path
#define compress_path z_compress_path
#define is_extention_equal z_is_extention_equal
#define dir_traverse z_dir_traverse
#define is_dir z_is_dir
#define is_regular_file z_is_regular_file
#define is_path_exists z_is_path_exists
#define write_file z_write_file
#define append_file z_append_file
#define read_file z_read_file
#define redirect_fd z_redirect_fd
#define popen2 z_popen2
#define mkdir z_mkdir
#define Cmd Z_Cmd
#define should_rebuild z_should_rebuild
#define should_rebuild_va z_should_rebuild_va
#define rebuild_yourself z_rebuild_yourself
#define cmd_init z_cmd_init
#define cmd_append z_cmd_append
#define cmd_append_va z_cmd_append_va
#define cmd_run_async z_cmd_run_async
#define run_async z_run_async
#define cmd_free z_cmd_free
#define cmd_clear z_cmd_clear

#endif // LIBZATAR_STRIP_PREFIX
#endif // LIBZATAR_STRIP_PREFIX_GAURD

//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
//       $       $       $       $       $       $        $        $
//   $       $       $       $       $       $       $        $        $
