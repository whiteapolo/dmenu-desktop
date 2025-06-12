#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/ucontext.h>
#include <unistd.h>

#define LIBZATAR_IMPLEMENTATION
#include "libzatar.h"

Z_MAP_DECLARE(Map, char *, char *, map)
Z_MAP_IMPLEMENT(Map, char *, char *, map)

const char *desktopAppsPath[] = {
	"/usr/share/applications",
	"~/.local/share/applications",
	"/var/lib/flatpak/exports/share/applications",
	NULL,
};

typedef struct {
	Z_String name;
	Z_String exec;
} DesktopFile;

void frees(char *s)
{
    free(s);
}

void die(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void removeFieldCodes(Z_String *s)
{
    Z_String tmp = {0};

	int i = 0;

	while (i < s->len) {
		if (s->ptr[i] == '%') {
            i += 2;
        } else {
            z_str_append_char(&tmp, s->ptr[i++]);
        }
	}

    z_str_clear(s);
    z_str_append_str(s, Z_STR_TO_SV(tmp));
    z_str_free(&tmp);
}

bool parseDesktopFile(const char *pathname, DesktopFile *desktopFile)
{
    Z_String file_content = {0};

    if (!z_read_whole_file(pathname, &file_content)) {
        return false;
    }

    bool has_name = false;
    bool has_exec = false;

    // initialize both strings
    memset(desktopFile, 0, sizeof(DesktopFile));

    Z_String_View delim = Z_CSTR_TO_SV("\n");
    Z_String_View line = z_str_tok_start(Z_STR_TO_SV(file_content), delim);

    while (line.len > 0 && (!has_name || !has_exec)) {
        if (!has_name && line.len > 5 && z_str_compare_n(line, Z_CSTR_TO_SV("Name="), 5) == 0) {
            desktopFile->name = z_str_new_format("%.*s", line.len - 5, line.ptr + 5);
            has_name = true;
        }

        if (!has_exec && line.len > 5 && z_str_compare_n(line, Z_CSTR_TO_SV("Exec="), 5) == 0) {
            desktopFile->exec = z_str_new_format("%.*s", line.len - 5, line.ptr + 5);
            has_exec = true;
        }

        line = z_str_tok_next(Z_STR_TO_SV(file_content), line, delim);
    }

    z_str_free(&file_content);
    z_str_trim(&desktopFile->name);
    z_str_trim(&desktopFile->exec);

    if (has_name && has_exec) {
        return true;
    }

    z_str_free(&(desktopFile->name));
    z_str_free(&desktopFile->exec);

	return false;
}

void proccessDesktopFile(const char *pathname, Map *programs)
{
	DesktopFile desktopFile = {0};

	if (parseDesktopFile(pathname, &desktopFile)) {
		removeFieldCodes(&desktopFile.exec);
        char *exec = desktopFile.exec.ptr;
        char *name = desktopFile.name.ptr;
		map_put(programs, name, exec, frees, frees);
	}
}

void processDirectory(const char *dirPath, Map *programs)
{
	DIR *dir = opendir(dirPath);

	if (dir == NULL)
		return;

    struct dirent *entry;

    Z_String full_path = {0};

    while ((entry = readdir(dir)) != NULL) {
        Z_String_View extention = z_get_path_extention(Z_CSTR_TO_SV(entry->d_name));
        if (z_str_compare(extention, Z_CSTR_TO_SV("desktop")) == 0) {
            z_str_clear(&full_path);
            z_str_append_format(&full_path, "%s/%s", dirPath, entry->d_name);
            proccessDesktopFile(full_path.ptr, programs);
        }
    }

    z_str_free(&full_path);
	closedir(dir);
}

Map processDirectories(const char *dirs[])
{
    Map programs = { NULL, strcmp };

    Z_String full_path = {0};

	for (int i = 0; dirs[i] != NULL; i++) {
        z_expand_path(Z_CSTR_TO_SV(dirs[i]), &full_path);
		processDirectory(z_str_to_cstr(&full_path), &programs);
        z_str_clear(&full_path);
	}

    z_str_free(&full_path);

    return programs;
}

int excuteProgram(const Map *programs, const char *programName)
{
    printf("selected program: '%s'\n", programName);

    char *command;

    if (map_find(programs, programName, &command)) {

        int pid = fork();

        if (pid == -1) {
            die("fork failed");
        }

        if (pid == 0) {
            printf("running: '%s'\n", command);
            z_redirect_fd(STDOUT_FILENO, "/dev/null");
            z_redirect_fd(STDERR_FILENO, "/dev/null");

            return system(command);
        }
    } else {
        fprintf(stderr, "Not a program: '%s'\n", programName);
    }

    return 0;
}

void printProgramName(char *key, char *data, void *arg)
{
    (void)data;
	fprintf((FILE *)arg, "%s\n", key);
}

int main(int argc, char **argv)
{
    (void)argc;

	FILE *pipe[2];
	argv[0] = "dmenu";

	// open dmenu in bidirectional pipe
	if (!z_popen2("dmenu", argv, pipe)) {
        die("popen2()");
    }

    Map programs = processDirectories(desktopAppsPath);

	// pipe program names to dmenu
    map_order_traverse(&programs, printProgramName, pipe[Z_Pipe_Mode_Write]);
	fclose(pipe[Z_Pipe_Mode_Write]);

	// read dmenu output
    Z_String program_name = {0};
    z_str_get_line(pipe[Z_Pipe_Mode_Read], &program_name);
    z_str_trim(&program_name);

    if (program_name.len > 0) {
        excuteProgram(&programs, z_str_to_cstr(&program_name));
    }

    z_str_free(&program_name);

	fclose(pipe[Z_Pipe_Mode_Read]);
    map_free(&programs, frees, frees);
}
