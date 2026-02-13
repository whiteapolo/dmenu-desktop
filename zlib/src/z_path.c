#include <z_path.h>
#include <z_env.h>

Z_String z_expand_tilde(Z_Heap *heap, Z_String_View pathname)
{
  if (z_sv_starts_with(pathname, z_sv_from_cstr("~"))) {
    Z_String expanded = z_str_new(heap, "%s", z_try_get_env("HOME", "."));
    z_str_append_str(&expanded, z_sv_advance(pathname, 1));
    return expanded;
  }

  return z_str_new_from_sv(heap, pathname);
}

Z_String z_compress_tilde(Z_Heap *heap, Z_String_View pathname)
{
  const char *home = z_try_get_env("HOME", NULL);

  if (home && z_sv_starts_with(pathname, z_sv_from_cstr(home))) {
    Z_String compressed = z_str_new(heap, "~");
    z_str_append_str(&compressed, z_sv_advance(pathname, strlen(home)));
    return compressed;
  }

  return z_str_new_from_sv(heap, pathname);
}
