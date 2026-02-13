#ifndef Z_HASH_TABLE
#define Z_HASH_TABLE

#include <stdbool.h>
#include <stdlib.h>
#include <z_compare.h>
#include <z_array.h>
#include <z_heap.h>

#define Z_HASH_TABLE_MIN_CAPACITY 16
#define Z_HASH_TABLE_MAX_LOAD_FACTOR 0.7
#define Z_HASH_TABLE_EMPTY 0
#define Z_HASH_TABLE_TOMBSTONE 1

typedef struct {
    void *key;
    void *value;
} Z_Pair;

typedef struct {
    bool ok;
    Z_Pair value;
} Z_Maybe_Pair;

Z_DEFINE_ARRAY(Z_Pair_Array, Z_Pair);

typedef size_t (*Z_Hash_Fn)(const void *);
typedef bool (*Z_Equal_Fn)(const void *, const void *);

typedef struct {
    void **keys;
    void **values;
    size_t *hashes;
    size_t occupied;
    size_t size;
    size_t capacity;
    Z_Equal_Fn equal;
    Z_Hash_Fn hash;
    Z_Heap *heap;
} Z_Hash_Table;

Z_Pair z_make_pair(void *key, void *value);
Z_Hash_Table z_hash_table_new(Z_Heap *heap, Z_Equal_Fn equal, Z_Hash_Fn hash);
Z_Hash_Table z_hash_table_new_with_capacity(Z_Heap *heap, Z_Equal_Fn equal, Z_Hash_Fn hash, size_t capacity);
void *z_hash_table_get(const Z_Hash_Table *table, void *key);
void *z_hash_table_try_get(const Z_Hash_Table *table, void *key, void *fallback);
Z_Maybe_Pair z_hash_table_put(Z_Hash_Table *table, void *key, void *value);
Z_Maybe_Pair z_hash_table_delete(Z_Hash_Table *table, void *key);
bool z_hash_table_contains(const Z_Hash_Table *table, void *key);
size_t z_hash_table_size(const Z_Hash_Table *table);
Z_Pair_Array z_hash_table_to_array(Z_Heap *heap, const Z_Hash_Table *table);

bool z_str_equal(const void *a, const void *b);
size_t z_str_hash(const void *s);

#endif
