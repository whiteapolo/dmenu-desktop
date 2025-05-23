#include <stdio.h>
#define LIBZATAR_IMPLEMENTATION
#include "libzatar.h"

#define TARGET "dmenu-desktop-z"
#define INSTALL_PATH "/usr/bin"

int build()
{
    Z_Cmd cmd;
    z_cmd_init(&cmd);
    z_cmd_append(&cmd, "cc", "main.c", "-o", TARGET);
    z_cmd_append(&cmd, "-Wextra", "-Wall", "-pedantic");

    return z_cmd_run_async(&cmd);
}

int clean()
{
    int status = remove(TARGET);

    if (status == 0) {
        z_print_info("removed '%s'", TARGET);
    }

    return status;
}

int install()
{
    return z_run_async("cp", TARGET, INSTALL_PATH);
}

int uninstall()
{
    int status = remove(INSTALL_PATH "/" TARGET);

    if (status == 0) {
        z_print_info("removed '%s'", TARGET);
    } else {
        z_print_error("unable to remove '%s': %s", TARGET, strerror(errno));
    }

    return status;
}


int main(int argc, char **argv)
{
    z_rebuild_yourself(__FILE__, "./build");

    if (argc == 1) {
        return build();
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "build") == 0) {
            build();
        } else if (strcmp(argv[i], "clean") == 0) {
            clean();
        } else if (strcmp(argv[i], "install") == 0) {
            install();
        } else if (strcmp(argv[i], "uninstall") == 0) {
            uninstall();
        } else {
            z_print_error("unrecognized option '%s'", argv[i]);
            z_print_error("possible options are: 'build', 'clean', 'install', 'uninstall'");
        }
    }

    return 0;
}
