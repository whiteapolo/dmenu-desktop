#include <stdlib.h>
#include <z_env.h>

const char *z_try_get_env(const char *name, const char *fallback)
{
    const char *value = getenv(name);
    return value ? value : fallback;
}