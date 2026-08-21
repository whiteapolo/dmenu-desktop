#include <z_string.h>
#include <z_array.h>
#include <limits.h>
#include <stdio.h>
#include <internal/z_math.h>

#define Z__WHITE_SPACE " \f\n\r\t\v"

int z__size_t_to_int(size_t a);
size_t z__get_format_length(const char *format, va_list args);

int z__size_t_to_int(size_t a)
{
    return a > INT_MAX ? INT_MAX : (int)a;
}

size_t z__get_format_length(const char *format, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    assert(size >= 0);

    return (size_t)size;
}

Z_String z_str_new(Z_Heap *heap, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Z_String s = z_str_new_args(heap, format, args);
    va_end(args);
    return s;
}

Z_String z_str_new_args(Z_Heap *heap, const char *format, va_list args)
{
    Z_String s = z_array_new(heap, Z_String);
    z_str_append_format_va(&s, format, args);
    return s;
}

Z_String z_str_new_from_sv(Z_Heap *heap, Z_String_View s)
{
    return z_str_new(heap, "%.*s", z__size_t_to_int(s.length), s.ptr);
}

char *z_sv_to_cstr(Z_Heap *heap, Z_String_View s)
{
    char *ret = z_heap_malloc(heap, sizeof(char) * (s.length + 1));
    memcpy(ret, s.ptr, sizeof(char) * s.length);
    ret[s.length] = 0;
    return ret;
}

char *z_cstr_dup(Z_Heap *heap, const char *s)
{
    return z_sv_to_cstr(heap, z_sv(s));
}

char *z_str_to_cstr(Z_String s)
{
    return s.ptr;
}

void z_str_append_cstr(Z_String *s, const char *cstr)
{
    z_str_append_str(s, z_sv(cstr));
}

void z_str_append_format(Z_String *s, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    z_str_append_format_va(s, format, args);
    va_end(args);
}

void z_str_append_format_va(Z_String *s, const char *format, va_list args)
{
    size_t format_length = z__get_format_length(format, args);
    z_array_ensure_capacity(s, s->length + format_length + 1);

    va_list args_copy;
    va_copy(args_copy, args);
    vsnprintf(s->ptr + s->length, format_length + 1, format, args_copy);
    va_end(args_copy);

    s->length += format_length;
}

void z_str_append_str(Z_String *target, Z_String_View source)
{
    z_array_ensure_capacity(target, target->length + source.length + 1);
    memcpy(target->ptr + target->length, source.ptr, sizeof(char) * source.length);
    target->length += source.length;
    z_array_zero_terminate(target);
}

void z_str_append_char(Z_String *s, char c)
{
    z_array_ensure_capacity(s, s->length + 2);
    s->ptr[s->length++] = c;
    z_array_zero_terminate(s);
}

bool z_str_append_file(Z_String *s, const char *pathname)
{
    FILE *fp = fopen(pathname, "r");

    if (fp == NULL) {
        return false;
    }

    size_t file_size = z__get_file_size(fp);

    z_array_ensure_capacity(s, s->length + file_size);
    s->length += fread(s->ptr + s->length, 1, file_size, fp);
    z_array_zero_terminate(s);
    fclose(fp);

    return true;
}


void z_str_prepend_format(Z_String *s, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    z_str_prepend_va(s, format, args);
    va_end(args);
}

void z_str_prepend_va(Z_String *s, const char *format, va_list args)
{
    Z_Heap_Auto heap = {0};
    Z_String tmp = z_str_new_args(&heap, format, args);
    z_str_append_format(&tmp, "%s", s->ptr);
    z_str_clear(s);
    z_str_append_format(s, "%s", tmp.ptr);
}

void z_str_prepend_str(Z_String *target, Z_String_View source)
{
    z_str_prepend_format(target, "%.*s", z__size_t_to_int(source.length), source.ptr);
}

void z_str_prepend_char(Z_String *s, char c)
{
    z_str_prepend_format(s, "%c", c);
}

char z_str_pop_char(Z_String *s)
{
    assert(s->length > 0);
    char c = z_array_pop(s);
    z_array_zero_terminate(s);
    return c;
}

void z_str_replace(Z_String *s, Z_String_View target, Z_String_View replacement)
{
    Z_Heap_Auto heap = {0};
    Z_String tmp = z_str_new(&heap, "");

    size_t i = 0;

    while (i < s->length) {
        if (z_sv_equal_n(z_sv_advance(z_sv(s), i), target, target.length)) {
            z_str_append_str(&tmp, replacement);
            i += target.length;
        } else {
            z_str_append_char(&tmp, s->ptr[i]);
            i++;
        }
    }

    z_str_clear(s);
    z_str_append_format(s, "%s", tmp.ptr);
}

Z_Sv_Split_Iter z_sv_split_iter(Z_String_View s, Z_String_View delimeter)
{
    Z_Sv_Split_Iter iter = {
        .s = s,
        .delimeter = delimeter,
        .current = 0,
        .is_done = false,
    };

    return iter;
}

bool z_sv_split_iter_next(Z_Sv_Split_Iter *iterator, Z_String_View *slice)
{
    if (iterator->current > iterator->s.length) {
        return false;
    }

    Z_String_View small_world = z_sv_advance(iterator->s, iterator->current);
    ssize_t next = z_sv_find_index(small_world, iterator->delimeter);

    if (next < 0) {
        iterator->current += small_world.length + 1;
        *slice = small_world;
        return true;
    }

    iterator->current += (size_t)next + iterator->delimeter.length;
    *slice = z_sv_substring(small_world, 0, (size_t)next);
    return true;
}

void z_str_split(Z_String_View s, Z_String_View delimiter, Z_String_Array *out)
{
    Z_Sv_Split_Iter iter = z_sv_split_iter(s, delimiter);
    Z_String_View curr;

    while (z_sv_split_iter_next(&iter, &curr)) {
        z_array_push(out, z_str_new_from_sv(out->heap, curr));
    }
}

Z_String_View z_sv_split_part(Z_String_View s, Z_String_View delimiter, size_t index)
{
    Z_Sv_Split_Iter iter = z_sv_split_iter(s, delimiter);
    Z_String_View slice;

    for (size_t i = 0; i < index; i++) {
        z_sv_split_iter_next(&iter, &slice);
    }

    assert(z_sv_split_iter_next(&iter, &slice) && "index of split is overreaching");
    return slice;
}

Z_String_View z_sv_from_str_ptr(const Z_String *s)
{
    Z_String_View view = {
        .ptr = s->ptr,
        .length = s->length
    };

    return view;
}

Z_String_View z_sv_from_str(Z_String s)
{
    Z_String_View view = {
        .ptr = s.ptr,
        .length = s.length
    };

    return view;
}

Z_String_View z_sv_from_cstr(const char *s)
{
    Z_String_View view = {
        .ptr = s,
        .length = strlen(s)
    };

    return view;
}

Z_String_View z_sv_advance(Z_String_View s, size_t offset)
{
    Z_String_View view = {
        .ptr = s.ptr + offset,
        .length = z__size_t_to_int(s.length) - z__size_t_to_int(offset) < 0 ? 0 : s.length - offset,
    };

    return view;
}

Z_String_View z_sv_substring(Z_String_View s, size_t start, size_t end)
{
    Z_String_View view = {
        .ptr = s.ptr + start,
        .length = end - start,
    };

    return view;
}

char z_sv_top(Z_String_View s)
{
    return s.ptr[s.length - 1];
}

int z_sv_compare(Z_String_View a, Z_String_View b)
{
    int result = memcmp(a.ptr, b.ptr, z__min_size_t(a.length, b.length));

    if (result == 0) {
        return z__size_t_to_int(a.length) - z__size_t_to_int(b.length);
    }

    return result;
}

bool z_sv_equal(Z_String_View a, Z_String_View b)
{
    return z_sv_compare(a, b) == 0;
}

bool z_sv_naive_like(Z_String_View str, Z_String_View pattern)
{
    if (str.length == 0 && pattern.length == 0) {
        return true;
    }

    if (str.length == 0 || pattern.length == 0) {
        return false;
    }

    if (pattern.ptr[0] == '%') {
        return z_sv_naive_like(z_sv_advance(str, 1), z_sv_advance(pattern, 1))
            || z_sv_naive_like(z_sv_advance(str, 1), pattern)
            || z_sv_naive_like(str, z_sv_advance(pattern, 1));
    }

    if (str.ptr[0] == pattern.ptr[0] || pattern.ptr[0] == '_') {
        return z_sv_naive_like(z_sv_advance(str, 1), z_sv_advance(pattern, 1));
    }

    return false;
}

bool z_sv_like(Z_String_View str, Z_String_View pattern)
{
    ssize_t last_wildcard_match = -1;
    ssize_t last_pattern_wildcard = -1;
    size_t i = 0;
    size_t j = 0;

    while (i < str.length && j < pattern.length) {
        if (pattern.ptr[j] == '%') {
            last_pattern_wildcard = (ssize_t)j;
            last_wildcard_match = (ssize_t)i;
            j++;
        } else if (str.ptr[i] == pattern.ptr[j] || pattern.ptr[j] == '_') {
            i++;
            j++;
        } else if (last_pattern_wildcard != -1) {
            j = (size_t)last_pattern_wildcard + 1;
            i = (size_t)++last_wildcard_match;
        } else {
            return false;
        }
    }

    if (i == str.length && j == pattern.length) {
        return true;
    }

    if ((j == pattern.length || j + 1 == pattern.length) && z_sv_top(pattern) == '%') {
        return true;
    }

    return false;
}

int z_sv_compare_n(Z_String_View a, Z_String_View b, size_t n)
{
    size_t length = z__min_size_t(n, z__min_size_t(a.length, b.length));
    int result = memcmp(a.ptr, b.ptr, length);

    if (result != 0) return result;
    if (n <= length) return 0;
    if (a.length < b.length) return -1;
    if (a.length > b.length) return 1;
    return 0;
}

bool z_sv_equal_n(Z_String_View a, Z_String_View b, size_t n)
{
    return z_sv_compare_n(a, b, n) == 0;
}

bool z_sv_ends_with(Z_String_View s, Z_String_View end)
{
    if (s.length < end.length) {
        return false;
    }

    Z_String_View endings = {
        .ptr = s.ptr + s.length - end.length,
        .length = end.length,
    };

    return z_sv_equal(endings, end);
}

bool z_sv_starts_with(Z_String_View s, Z_String_View start)
{
    if (start.length > s.length) {
        return false;
    }

    return z_sv_equal_n(s, start, start.length);
}

bool z_sv_contains(Z_String_View haystack, Z_String_View needle)
{
    return z_sv_find_index(haystack, needle) != -1;
}

bool z_sv_contain_char(Z_String_View s, char c)
{
    return memchr(s.ptr, c, s.length);
}

ssize_t z_sv_find_index(Z_String_View haystack, Z_String_View needle)
{
    if (needle.length == 0) {
        return 0;
    }

    if (needle.length > haystack.length) {
        return -1;
    }

    for (size_t i = 0; i < haystack.length - needle.length + 1; i++) {
        if (memcmp(haystack.ptr + i, needle.ptr, needle.length) == 0) {
            return (ssize_t)i;
        }
    }

    return -1;
}

void z_str_trim(Z_String *s)
{
    z_str_trim_right(s);
    z_str_trim_left(s);
}

void z_str_trim_cset(Z_String *s, Z_String_View cset)
{
    z_str_trim_right_cset(s, cset);
    z_str_trim_left_cset(s, cset);
}

void z_str_trim_right(Z_String *s)
{
    z_str_trim_right_cset(s, z_sv(Z__WHITE_SPACE));
}

void z_str_trim_left(Z_String *s)
{
    z_str_trim_left_cset(s, z_sv(Z__WHITE_SPACE));
}

void z_str_trim_right_cset(Z_String *s, Z_String_View cset)
{
    Z_String_View trimmed = z_sv_trim_right_cset(z_sv(s), cset);
    memmove(s->ptr, trimmed.ptr, trimmed.length);
    s->length = trimmed.length;
    z_array_zero_terminate(s);
}

void z_str_trim_left_cset(Z_String *s, Z_String_View cset)
{
    Z_String_View trimmed = z_sv_trim_left_cset(z_sv(s), cset);
    memmove(s->ptr, trimmed.ptr, trimmed.length);
    s->length = trimmed.length;
    z_array_zero_terminate(s);
}

Z_String_View z_sv_trim(Z_String_View s)
{
    return z_sv_trim_cset(s, z_sv(Z__WHITE_SPACE));
}

Z_String_View z_sv_trim_cset(Z_String_View s, Z_String_View cset)
{
    Z_String_View trimmed = z_sv_trim_right_cset(s, cset);
    return z_sv_trim_left_cset(trimmed, cset);
}

Z_String_View z_sv_trim_right(Z_String_View s)
{
    return z_sv_trim_right_cset(s, z_sv(Z__WHITE_SPACE));
}

Z_String_View z_sv_trim_right_cset(Z_String_View s, Z_String_View cset)
{
    Z_String_View trimmed = s;

    while (trimmed.length > 0 && z_sv_contain_char(cset, z_sv_top(trimmed))) {
        trimmed.length--;
    }

    return trimmed;
}

Z_String_View z_sv_trim_left(Z_String_View s)
{
    return z_sv_trim_left_cset(s, z_sv(Z__WHITE_SPACE));
}

Z_String_View z_sv_trim_left_cset(Z_String_View s, Z_String_View cset)
{
    Z_String_View trimmed = s;

    while (trimmed.length > 0 && z_sv_contain_char(cset, s.ptr[0])) {
        trimmed = z_sv_advance(trimmed, 1);
    }

    return trimmed;
}

void z_sv_print(Z_String_View s)
{
    printf("%.*s", z__size_t_to_int(s.length), s.ptr);
}

void z_sv_println(Z_String_View s)
{
    printf("%.*s\n", z__size_t_to_int(s.length), s.ptr);
}

void z_str_clear(Z_String *s)
{
    s->length = 0;
    z_array_zero_terminate(s);
}

void z_str_set_format(Z_String *s, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    z_str_set_format_va(s, format, args);
    va_end(args);
}

void z_str_set_format_va(Z_String *s, const char *format, va_list args)
{
    z_str_clear(s);
    z_str_append_format_va(s, format, args);
}
