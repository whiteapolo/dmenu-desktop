#include <z_heap.h>
#include <internal/z_math.h>
#include <stdbool.h>

#define Z_PTR_TABLE_MIN_CAPACITY 16
#define Z_PTR_TABLE_MAX_LOAD_FACTOR 0.7
#define Z_PTR_TABLE_TOMBSTONE ((uintptr_t)1)
#define Z_PTR_TABLE_EMPTY ((uintptr_t)0)

static inline size_t z__ptr_table_fast_mod(size_t value, size_t mod)
{
    return value & (mod - 1);
}

static inline uintptr_t z__ptr_table_hash(uintptr_t ptr)
{
    return ptr;
}

static inline float z__ptr_table_load_factor(const Z_Ptr_Table *table)
{
    if (table->capacity == 0) {
        return 1;
    }

    return (float)table->occupied / (float)table->capacity;
}

void z__ptr_table_insert_no_check(Z_Ptr_Table *table, uintptr_t ptr)
{
    size_t i = z__ptr_table_fast_mod(z__ptr_table_hash(ptr), table->capacity);

    while (table->ptr[i] > 1) {
        i = z__ptr_table_fast_mod(i + 1, table->capacity);
    }

    if (table->ptr[i] == Z_PTR_TABLE_EMPTY) {
        table->occupied++;
    }

    table->ptr[i] = ptr;
}

void z__ptr_table_resize(Z_Ptr_Table *table, size_t new_capacity)
{
    Z_Ptr_Table new = {
        .ptr = calloc(new_capacity, sizeof(uintptr_t)),
        .occupied = 0,
        .capacity = new_capacity,
    };

    for (size_t i = 0; i < table->capacity; i++) {
        if (table->ptr[i] > 1) {
            z__ptr_table_insert_no_check(&new, table->ptr[i]);
        }
    }

    free(table->ptr);
    *table = new;
}

void z_ptr_table_insert(Z_Ptr_Table *table, uintptr_t ptr)
{
    if (z__ptr_table_load_factor(table) >= Z_PTR_TABLE_MAX_LOAD_FACTOR) {
        size_t new_capacity = z__max_size_t(Z_PTR_TABLE_MIN_CAPACITY, table->capacity * 2);
        z__ptr_table_resize(table, new_capacity);
    }

    z__ptr_table_insert_no_check(table, ptr);
}

void z_ptr_table_delete(Z_Ptr_Table *table, uintptr_t ptr)
{
    size_t i = z__ptr_table_fast_mod(z__ptr_table_hash(ptr), table->capacity);

    while (table->ptr[i] != Z_PTR_TABLE_EMPTY) {
        if (table->ptr[i] == ptr) {
            table->ptr[i] = Z_PTR_TABLE_TOMBSTONE;
            return;
        }

        i = z__ptr_table_fast_mod(i + 1, table->capacity);
    }
}

void z_ptr_table_free(Z_Ptr_Table *table)
{
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->ptr[i] > 1) {
            free((void*)table->ptr[i]);
        }
    }

    free(table->ptr);
}

void z_ptr_table_reset(Z_Ptr_Table *table)
{
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->ptr[i] > 1) {
            free((void*)table->ptr[i]);
        }
    }

    memset(table->ptr, 0, sizeof(uintptr_t) * table->capacity);
    table->occupied = 0;
}

void *z_heap_malloc(Z_Heap *heap, size_t size)
{
   void *ptr = malloc(size);
   z_ptr_table_insert(heap, (uintptr_t)ptr);
   return ptr;
}

void *z_heap_calloc(Z_Heap *heap, size_t size)
{
   void *ptr = calloc(1, size);
   z_ptr_table_insert(heap, (uintptr_t)ptr);
   return ptr;
}

void *z_heap_realloc(Z_Heap *heap, void *ptr, size_t new_size)
{
    void *new_ptr = realloc(ptr, new_size);

    if (ptr != NULL) {
        z_ptr_table_delete(heap, (uintptr_t)ptr);
    }

    if (ptr != new_ptr) {
        z_ptr_table_insert(heap, (uintptr_t)new_ptr);
    }

    return new_ptr;
}

void z_heap_free(Z_Heap *heap, void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    free(ptr);
    z_ptr_table_delete(heap, (uintptr_t)ptr);
}

void z_heap_free_all(Z_Heap *heap)
{
    z_ptr_table_free(heap);
}

void z_heap_reset(Z_Heap *heap)
{
    z_ptr_table_reset(heap);
}
