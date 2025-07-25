#include <stdio.h>
#define LIBZATAR_IMPLEMENTATION
#include "libzatar.h"

typedef void (*voidFn)();

#define TARGET "dmenu-desktop"
#define INSTALL_PATH "/usr/local/bin"

int build() {
  if (!z_should_rebuild(TARGET, "main.c", "libzatar.h")) {
    return 0;
  }

  Z_Cmd cmd = {0};
  z_cmd_append(&cmd, "cc", "main.c", "-o", TARGET);
  z_cmd_append(&cmd, "-Wextra", "-Wall", "-pedantic");
  z_cmd_append(&cmd, "-O3");

  return z_cmd_run_async(&cmd);
}

void clean() {
  int status = remove(TARGET);

  if (status == 0) {
    z_print_info("removed '%s'", TARGET);
  }
}

void install() {
  int status = build();

  if (status == 0) {
    Z_Cmd cmd = {0};
    z_cmd_append(&cmd, "cp", TARGET, INSTALL_PATH);
    z_cmd_run_async(&cmd);
    z_cmd_free(&cmd);
  }
}

void uninstall() {
  int status = remove(INSTALL_PATH "/" TARGET);

  if (status == 0) {
    z_print_info("removed '%s'", TARGET);
  } else {
    z_print_error("unable to remove '%s': %s", TARGET, strerror(errno));
  }
}

void prints(char *s, voidFn f, void *arg) {
  (void)f;
  (void)arg;
  printf("'%s'  ", s);
}

int main(int argc, char **argv) {
  z_rebuild_yourself(__FILE__, argv);

  if (argc == 1) {
    return build();
  }

  Z_Map params = {.compare_keys = (Z_Compare_Fn)strcmp};

  z_map_put(&params, "clean", clean, NULL, NULL);
  z_map_put(&params, "install", install, NULL, NULL);
  z_map_put(&params, "uninstall", uninstall, NULL, NULL);

  for (int i = 1; i < argc; i++) {

    voidFn f = z_map_get(&params, argv[i]);

    if (f) {
      f();
    } else {
      z_print_error("unrecognized option '%s'", argv[i]);
      z_print_error("possible options are: ");
      z_map_order_traverse(&params, (void (*)(void *, void *, void *))prints,
                           NULL);
      printf("\n");
    }
  }

  return 0;
}
