#include <stdio.h>
#define LIBZATAR_IMPLEMENTATION
#include "libzatar.h"

#define TARGET "dmenu-desktop"
#define INSTALL_PATH "/usr/local/bin"

int build()
{
    Z_Cmd cmd;
    z_cmd_init(&cmd);
    z_cmd_append(&cmd, "cc", "main.c", "-o", TARGET);
    z_cmd_append(&cmd, "-Wextra", "-Wall", "-pedantic");
    z_cmd_append(&cmd, "-O3");

    return z_cmd_run_async(&cmd);
}

int install()
{
    int status = build();

    if (status == 0) {
        return z_run_async("cp", TARGET, INSTALL_PATH);
    }

    return status;
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
    z_rebuild_yourself(__FILE__, argv);

    if (argc == 1) {
        return build();
    } else if (argc == 2) {
        if (strcmp(argv[1], "install") == 0) {
            return install();
        } else if (strcmp(argv[1], "uninstall") == 0) {
            return uninstall();
        } else {
            z_print_error("unrecognized option '%s'", argv[1]);
            z_print_error("possible options are: 'install', 'uninstall'");
            return -1;
        }
    }

    return 0;
}
