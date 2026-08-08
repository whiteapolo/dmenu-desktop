#include <z_path.h>
#include <z_env.h>

bool z_expand_tilde(Z_String_View pathname, Z_String *out)
{
    if (!z_sv_starts_with(pathname, z_sv("~"))) {
        z_str_append_str(out, pathname);
        return false;
    }

    z_str_append_cstr(out, z_try_get_env("HOME", "."));
    z_str_append_str(out, z_sv_advance(pathname, 1));
    return true;
}

bool z_compress_tilde(Z_String_View pathname, Z_String *out)
{
    const char *home = z_try_get_env("HOME", ".");

    if (!z_sv_starts_with(pathname, z_sv(home))) {
        z_str_append_str(out, pathname);
        return false;
    }
    
    z_str_append_cstr(out, "~");
    z_str_append_str(out, z_sv_advance(pathname, strlen(home)));
    return true;
}
