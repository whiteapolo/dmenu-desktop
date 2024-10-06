#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#define PATH_IMPL
#include "external/path.h"

#define MAP_IMPL
#include "external/map.h"

const char *desktopAppsPath[] = {
	"/usr/share/applications",
	"~/.local/share/applications",
	"/var/lib/flatpak/exports/share/applications",
	NULL,
};

typedef struct {
	char *name;
	char *exec;
} DesktopFile;

void die(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void removeFieldCodes(char *s)
{
	while (*(s++)) {
		if (*s == '%') {
			memset(s, ' ', 2);
			s += 2;
		}
	}
}

char strpoplast(char *s)
{
	char *end = s + strlen(s) - 1;
	char c = *end;
	*end = '\0';
	return c;
}

int parseDesktopFile(const char *fileName, DesktopFile *desktopFile)
{
	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		return Err;

	char *line = NULL;
	size_t lineLen = 0;
	desktopFile->name = NULL;
	desktopFile->exec = NULL;

	while (getline(&line, &lineLen, fp) > 0) {
		strpoplast(line); // remove new line
		if (desktopFile->name == NULL && strncmp(line, "Name=", 5) == 0)
			desktopFile->name = strdup(line + 5);
		else if (desktopFile->exec == NULL && strncmp(line, "Exec=", 5) == 0)
			desktopFile->exec = strdup(line + 5);
	}

	free(line);
	fclose(fp);

	if (desktopFile->name && desktopFile->exec)
		return Ok;
	free(desktopFile->name);
	free(desktopFile->exec);
	errno = EINVAL;
	return Err;
}

void proccessDesktopFile(const char *fileName, map *indexMap)
{
	DesktopFile desktopFile;

	if (parseDesktopFile(fileName, &desktopFile) == Ok) {
		removeFieldCodes(desktopFile.exec);
		mapInsert(indexMap, desktopFile.name, desktopFile.exec);
	}

	// no need to free desktopFile because we send it to mapInsert
}

void processDirectory(const char *dirPath, map *indexMap)
{
	DIR *dir = opendir(dirPath);

	if (dir == NULL)
		return;

	char fileName[PATH_MAX];
	while (nextInDir(dir, dirPath, fileName, sizeof(fileName)) == Ok)
		if (isExtentionEqual(fileName, "desktop"))
			proccessDesktopFile(fileName, indexMap);

	closedir(dir);
}

map processDirectories(const char *dirs[])
{
	map programsMap = newMap((cmpKeysType)strcmp);
	for (int i = 0; dirs[i] != NULL; i++) {
		char fullDirName[PATH_MAX];
		snprintf(fullDirName, PATH_MAX, "%s", dirs[i]);
		expandPath(fullDirName, PATH_MAX);
		processDirectory(fullDirName, &programsMap);
	}
	return programsMap;
}

Result excuteProgram(const map programsMap, const char *programName)
{
	const char *command = (const char *)mapFind(programsMap, programName);

	if (command != NULL && fork() == 0) {
		redirectFd(STDOUT_FILENO, "/dev/null");
		redirectFd(STDERR_FILENO, "/dev/null");
		return system(command);
	}
	return Err;
}

void printProgramName(const void *key, const void *data, va_list ap)
{
	(void)data;
	FILE *out = va_arg(ap, FILE*);
	fprintf(out, "%s\n", (const char *)key);
}

int main(int, char **argv)
{
	FILE *pipe[2];
	argv[0] = "dmenu";

	// open dmenu in bidirectional pipe
	if (popen2("dmenu", argv, pipe) == Err)
		die("popen2()");

	// pipe program names to dmenu
	map programsMap = processDirectories(desktopAppsPath);
	mapOrderTraverse(programsMap, printProgramName, pipe[Write]);
	fclose(pipe[Write]);

	// read dmenu output
	char *buf = NULL;
	size_t bufLen = 0;
	if (getline(&buf, &bufLen, pipe[Read]) > 0) {
		strpoplast(buf); // remove new line
		excuteProgram(programsMap, buf);
	}

	free(buf);
	fclose(pipe[Read]);
	mapFree(programsMap, free, free);
}
