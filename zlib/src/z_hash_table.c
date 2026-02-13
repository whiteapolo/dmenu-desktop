#include <z_hash_table.h>

Z_Pair z_make_pair(void *key, void *value)
{
  Z_Pair pair = {
    .key = key,
    .value = value,
  };

  return pair;
}

Z_Hash_Table z_hash_table_new(Z_Heap *heap, Z_Equal_Fn equal, Z_Hash_Fn hash)
{
    return z_hash_table_new_with_capacity(heap, equal, hash, 0);
}

Z_Hash_Table z_hash_table_new_with_capacity(Z_Heap *heap, Z_Equal_Fn equal, Z_Hash_Fn hash, size_t capacity)
{
    Z_Hash_Table table = {
        .keys = z_heap_calloc(heap, sizeof(void *) * capacity),
        .values = z_heap_calloc(heap, sizeof(void *) * capacity),
        .hashes = z_heap_calloc(heap, sizeof(size_t) * capacity),
        .occupied = 0,
        .size = 0,
        .capacity = capacity,
        .equal = equal,
        .hash = hash,
        .heap = heap,
    };

    return table;
}

void z__hash_table_free(Z_Hash_Table *table)
{
  if (table->capacity == 0) {
    return;
  }

  z_heap_free(table->heap, table->keys);
  z_heap_free(table->heap, table->values);
  z_heap_free(table->heap, table->hashes);
}

static inline size_t z__hash_table_hash(const Z_Hash_Table *table, void *key)
{
    size_t hash = table->hash(key);

    if (hash < 2) {
        return 2;
    }

    return hash * 11;
}

static inline float z__hash_table_get_load_factor(const Z_Hash_Table *table)
{
    if (table->capacity == 0) {
        return 1;
    }

    return (float)table->occupied / (float)table->capacity;
}

void *z_hash_table_try_get(const Z_Hash_Table *table, void *key, void *fallback)
{
    if (table->capacity == 0) {
      return fallback;
    }

    size_t hash = z__hash_table_hash(table, key);
    size_t i = hash % table->capacity;

    while (table->hashes[i] != Z_HASH_TABLE_EMPTY) {

        if (table->hashes[i] == hash && table->equal(table->keys[i], key)) {
            return table->values[i];
        }

        i = (i + 1) % table->capacity;
    }

    return fallback;
}

void *z_hash_table_get(const Z_Hash_Table *table, void *key)
{
    return z_hash_table_try_get(table, key, NULL);
}

Z_Maybe_Pair z__hash_table_put_no_resize(Z_Hash_Table *table, void *key, void *value, size_t hash)
{
    size_t i = hash & (table->capacity - 1);
    ssize_t first_tompstone = -1;

    while (table->hashes[i] != Z_HASH_TABLE_EMPTY) {

        if (first_tompstone == -1 && table->hashes[i] == Z_HASH_TABLE_TOMBSTONE) {
            first_tompstone = i;
        }

        if (table->hashes[i] == hash && table->equal(table->keys[i], key)) {
            Z_Pair old = z_make_pair(table->keys[i], table->values[i]);
            table->keys[i] = key;
            table->values[i] = value;
            return (Z_Maybe_Pair){ .ok = true, .value = old };
        }

        i = (i + 1) % table->capacity;
    }

    if (first_tompstone == -1) {
      table->keys[i] = key;
      table->values[i] = value;
      table->hashes[i] = hash;
      table->occupied++;
    } else {
      table->keys[first_tompstone] = key;
      table->values[first_tompstone] = value;
      table->hashes[first_tompstone] = hash;
    }

    table->size++;

    return (Z_Maybe_Pair){ .ok = false };
}

void z__hash_table_resize(Z_Hash_Table *table, size_t new_capacity)
{
    Z_Hash_Table new_table = z_hash_table_new_with_capacity(table->heap, table->equal, table->hash, new_capacity);

    for (size_t i = 0; i < table->capacity; i++) {
        if (table->hashes[i] >= 2) {
            z__hash_table_put_no_resize(&new_table, table->keys[i], table->values[i], table->hashes[i]);
        }
    }

    z__hash_table_free(table);
    *table = new_table;
}

Z_Maybe_Pair z_hash_table_put(Z_Hash_Table *table, void *key, void *value)
{
    if (z__hash_table_get_load_factor(table) >= Z_HASH_TABLE_MAX_LOAD_FACTOR) {
        size_t new_capacity = z__max_size_t(Z_HASH_TABLE_MIN_CAPACITY, table->capacity * 2);
        z__hash_table_resize(table, new_capacity);
    }

    size_t hash = z__hash_table_hash(table, key);
    return z__hash_table_put_no_resize(table, key, value, hash);
}

Z_Maybe_Pair z_hash_table_delete(Z_Hash_Table *table, void *key)
{
    size_t hash = z__hash_table_hash(table, key);
    size_t i = hash % table->capacity;

    while (table->hashes[i] != Z_HASH_TABLE_EMPTY && (table->hashes[i] != hash || !table->equal(key, table->keys[i]))) {
        i = (i + 1) % table->capacity;
    }

    if (table->hashes[i] == Z_HASH_TABLE_EMPTY) {
        return (Z_Maybe_Pair){ .ok = false };
    }

    table->hashes[i] = Z_HASH_TABLE_TOMBSTONE;
    table->size--;

    return (Z_Maybe_Pair){ .ok = true, .value = z_make_pair(table->keys[i], table->values[i]) };
}

bool z_hash_table_contains(const Z_Hash_Table *table, void *key)
{
    if (table->capacity == 0) {
      return false;
    }

    size_t hash = z__hash_table_hash(table, key);
    size_t i = hash % table->capacity;

    while (table->hashes[i] != Z_HASH_TABLE_EMPTY) {
        if (table->hashes[i] == hash && table->equal(key, table->keys[i])) {
            return true;
        }

        i = (i + 1) % table->capacity;
    }

    return false;
}

size_t z_hash_table_size(const Z_Hash_Table *table)
{
    return table->size;
}

Z_Pair_Array z_hash_table_to_array(Z_Heap *heap, const Z_Hash_Table *table)
{
    Z_Pair_Array array = z_array_new(heap, Z_Pair_Array);

    for (size_t i = 0; i < table->capacity; i++) {
        if (table->hashes[i] >= 2) {
            z_array_push(&array, z_make_pair(table->keys[i], table->values[i]));
        }
    }

    return array;
}

bool z_str_equal(const void *a, const void *b)
{
    return strcmp(a, b) == 0;
}

size_t z_str_hash(const void *s)
{
    size_t hash = 5381;

    for (const char *curr = s; *curr != '\0'; curr++) {
        hash = ((hash << 5) + hash) + *curr;
    }

    return hash;
}
