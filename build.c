#define LIBZATAR_IMPLEMENTATION
#include "libzatar.h"

int main(int argc, char **argv)
{
    z_rebuild_yourself(__FILE__, "./build");

    Z_Cmd cmd;
    z_cmd_init(&cmd);

    z_cmd_append(&cmd, "cc", "main.c", "-o", "dmenu-desktop");
    z_cmd_append(&cmd, "-Wextra", "-Wall", "-pedantic");

    z_cmd_run_async(&cmd);

    return 0;
}
