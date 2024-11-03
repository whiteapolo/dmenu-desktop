#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "libzatar/include/str.h"
#include "libzatar/include/map.h"
#include "libzatar/include/path.h"

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

char strpoplast(char *s)
{
	char *end = s + strlen(s) - 1;
	char c = *end;
	*end = '\0';
	return c;
}

void strtrim(char *s)
{
	if (*s == '\0')
		return;

	char *end = s + strlen(s) - 1;
	while (end > s && isspace(*end))
		end--;
	*(end + 1) = '\0';
}

void removeFieldCodes(char *s)
{
	char *curr = s;
	while (*curr != '\0') {
		if (*curr == '%') {
			memset(curr, ' ', 2);
			curr++;
		}
		curr++;
	}
	strtrim(s);
}

int parseDesktopFile(const char *fileName, DesktopFile *desktopFile)
{
	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		return Err;

	Scanner scanner = newScanner(fp);
	desktopFile->name = NULL;
	desktopFile->exec = NULL;
	const string *line;
	while ((line = scannerNextLine(&scanner)) != NULL) {
		const char *lineptr = line->data;
		if (desktopFile->name == NULL && strncmp(lineptr, "Name=", 5) == 0)
			desktopFile->name = strdup(lineptr + 5);
		else if (desktopFile->exec == NULL && strncmp(lineptr, "Exec=", 5) == 0)
			desktopFile->exec = strdup(lineptr + 5);
	}

	scannerFree(&scanner);
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

Result excuteProgram(const map *programsMap, const char *programName)
{
	const char *command = (const char *)mapFind(programsMap, programName);

	if (command != NULL && fork() == 0) {
		redirectFd(STDOUT_FILENO, "/dev/null");
		redirectFd(STDERR_FILENO, "/dev/null");
		return system(command);
	}
	return Err;
}

void printProgramName(const void *key, const void *data, void *arg)
{
	(void)data;
	fprintf((FILE *)arg, "%s\n", (const char *)key);
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
	mapOrderTraverse(&programsMap, printProgramName, pipe[Write]);
	fclose(pipe[Write]);

	// read dmenu output
	char *buf = NULL;
	size_t bufLen = 0;
	if (getline(&buf, &bufLen, pipe[Read]) > 0) {
		strpoplast(buf); // remove new line
		excuteProgram(&programsMap, buf);
	}

	free(buf);
	fclose(pipe[Read]);
	mapFree(&programsMap, free, free);
}
