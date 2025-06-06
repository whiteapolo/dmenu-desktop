#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define LIBZATAR_IMPLEMENTATION
#include "libzatar.h"

Z_MAP_DECLARE(Map, Z_Str, Z_Str, map)
Z_MAP_IMPLEMENT(Map, Z_Str, Z_Str, map)

const char *desktopAppsPath[] = {
	"/usr/share/applications",
	"~/.local/share/applications",
	"/var/lib/flatpak/exports/share/applications",
	NULL,
};

typedef struct {
	Z_Str name;
	Z_Str exec;
} DesktopFile;

void die(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void removeFieldCodes(Z_Str *s)
{
    Z_Str tmp = z_str_new("");

	int i = 0;

	while (i < s->len) {
		if (s->ptr[i] == '%') {
            i += 2;
        } else {
            z_str_push_c(&tmp, s->ptr[i++]);
        }
	}

    z_str_free_ptr(s);
    *s = tmp;
}

Z_Result parseDesktopFile(const char *pathname, DesktopFile *desktopFile)
{
    Z_Str f;

    if (z_read_whole_file(&f, pathname) == Z_Err) {
        return Z_Err;
    }

    Z_Str_Slice line = z_str_tok_init(f);

    bool has_name = false;
    bool has_exec = false;

    while (z_str_tok_next(f, &line, "\n") == Z_Ok && (!has_name || !has_exec)) {
        if (!has_name && memcmp(line.ptr, "Name=", 5) == 0) {
            desktopFile->name = z_str_new("%.*s", line.len - 5, line.ptr + 5);
            has_name = true;
        }

        if (!has_exec && memcmp(line.ptr, "Exec=", 5) == 0) {
            desktopFile->exec = z_str_new("%.*s", line.len - 5, line.ptr + 5);
            has_exec = true;
        }
    }

    z_str_free(f);

    if (has_name && has_exec) {
        return Z_Ok;
    }

    if (has_name) {
        z_str_free(desktopFile->name);
    }

    if (has_exec) {
        z_str_free(desktopFile->exec);
    }

	return Z_Err;
}

void proccessDesktopFile(const char *pathname, Map *programs)
{
	DesktopFile desktopFile;

	if (parseDesktopFile(pathname, &desktopFile) == Z_Ok) {
		removeFieldCodes(&desktopFile.exec);
        // TODO: implement trim
        // z_str_trim(&desktopFile.exec);
        // z_str_trim(&desktopFile.name);
		map_put(programs, desktopFile.name, desktopFile.exec, z_str_free, z_str_free);
	}
}

void processDirectory(const char *dirPath, Map *programs)
{
	DIR *dir = opendir(dirPath);

	if (dir == NULL)
		return;

    struct dirent *entry;

    Z_Str full_path = z_str_new("");

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(z_get_path_extention(entry->d_name), "desktop") == 0) {
            z_str_clear(&full_path);
            z_str_push(&full_path, "%s/%s", dirPath, entry->d_name);
            proccessDesktopFile(full_path.ptr, programs);
        }
    }

    z_str_free(full_path);
	closedir(dir);
}

Map processDirectories(const char *dirs[])
{
    Map programs;
    map_init(&programs, z_str_cmp);

    Z_Str full_path;
    z_str_init(&full_path, "");

	for (int i = 0; dirs[i] != NULL; i++) {
        z_str_clear(&full_path);
		z_expand_path(dirs[i], &full_path);
		processDirectory(full_path.ptr, &programs);
	}

    z_str_free(full_path);

    return programs;
}

int excuteProgram(const Map *programs, Z_Str_Slice programName)
{
    Z_Str command = {0};

    printf("Selected: '%.*s'\n", programName.len, programName.ptr);

    if (map_find(programs, programName, &command)) {

        int pid = fork();

        if (pid == -1) {
            die("fork failed");
        }

        if (pid == 0) {
            printf("CMD: '%s'\n", command.ptr);
            z_redirect_fd(STDOUT_FILENO, "/dev/null");
            z_redirect_fd(STDERR_FILENO, "/dev/null");

            return system(command.ptr);
        }
    }

    return 0;
}

void printProgramName(Z_Str key, Z_Str data, void *arg)
{
    (void)data;
	fprintf((FILE *)arg, "%s\n", key.ptr);
}

int main(int argc, char **argv)
{
    (void)argc;

	FILE *pipe[2];
	argv[0] = "dmenu";

	// open dmenu in bidirectional pipe
	if (z_popen2("dmenu", argv, pipe) == Z_Err) {
        die("popen2()");
    }

    Map programs = processDirectories(desktopAppsPath);

	// pipe program names to dmenu
    map_order_traverse(&programs, printProgramName, pipe[Z_Pipe_Mode_Write]);
	fclose(pipe[Z_Pipe_Mode_Write]);

	// read dmenu output
    Z_Str program_name = z_str_get_line(pipe[Z_Pipe_Mode_Read]);

    if (!z_str_is_empty(program_name)) {
        excuteProgram(&programs, program_name);
    }

    z_str_free(program_name);

	fclose(pipe[Z_Pipe_Mode_Read]);
    map_free(&programs, z_str_free, z_str_free);
}
