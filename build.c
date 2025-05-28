#include <stdio.h>
#define LIBZATAR_IMPLEMENTATION
#include "libzatar.h"

typedef void (*voidFunc)();

Z_MAP_DECLARE(Map, char *, voidFunc, map)
Z_MAP_IMPLEMENT(Map, char *, voidFunc, map)

#define TARGET "dmenu-desktop"
#define INSTALL_PATH "/usr/local/bin"

int build()
{
    if (!z_should_rebuild(TARGET, "main.c")) {
        return 0;
    }

    Z_Cmd cmd;
    z_cmd_init(&cmd);
    z_cmd_append(&cmd, "cc", "main.c", "-o", TARGET);
    z_cmd_append(&cmd, "-Wextra", "-Wall", "-pedantic");
    z_cmd_append(&cmd, "-O3");

    return z_cmd_run_async(&cmd);
}

void clean()
{
    int status = remove(TARGET);

    if (status == 0) {
        z_print_info("removed '%s'", TARGET);
    } else {
        z_print_error("unable to remove '%s': %s", TARGET, strerror(errno));
    }
}

void install()
{
    int status = build();

    if (status == 0) {
        z_run_async("cp", TARGET, INSTALL_PATH);
    }
}

void uninstall()
{
    int status = remove(INSTALL_PATH "/" TARGET);

    if (status == 0) {
        z_print_info("removed '%s'", TARGET);
    } else {
        z_print_error("unable to remove '%s': %s", TARGET, strerror(errno));
    }
}

void prints(char *s, voidFunc f, void *arg)
{
    (void)f;
    (void)arg;
    printf("'%s'  ", s);
}

int main(int argc, char **argv)
{
    z_rebuild_yourself(__FILE__, argv);

    if (argc == 1) {
        return build();
    }

    Map params;
    map_init(&params, (int (*)(char*, char*)) strcmp);

    map_put(&params, "clean", clean, NULL, NULL);
    map_put(&params, "install", install, NULL, NULL);
    map_put(&params, "uninstall", uninstall, NULL, NULL);

    for (int i = 1; i < argc; i++) {

        voidFunc f;

        if (map_find(&params, argv[i], &f)) {
            f();
        } else {
            z_print_error("unrecognized option '%s'", argv[i]);
            z_print_error("possible options are: ");
            map_order_traverse(&params, prints, NULL);
            printf("\n");
        }
    }

    return 0;
}
