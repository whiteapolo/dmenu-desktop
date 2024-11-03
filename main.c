#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "libzatar/include/shared.h"
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
	string name;
	string exec;
} DesktopFile;

int cmpString(const void *a, const void *b)
{
	const string *s1 = a;
	const string *s2 = b;
	return strCmp(*s1, *s2);
}

void freeStr(void *s)
{
	string *str = s;
	strFree(str);
}

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

void removeFieldCodes(string *s)
{
	for (size_t i = 0; i < s->len; i++)
		if (s->data[i] == '%')
			memset(&s->data[i], ' ', 2);
	strTrim(s);
}

int parseDesktopFile(const char *fileName, DesktopFile *desktopFile)
{
	string f = readWholeFile(fileName);
	if (strIsEmpty(f))
		return Err;
	strSlice line = strTokStart(f, "\n");

	desktopFile->name = newStr("");
	desktopFile->exec = newStr("");

	while (!strIsEmpty(line)) {
		if (strIsEmpty(desktopFile->name) && strnIsEqualC(line, "Name=", 5))
			desktopFile->name = newStrSlice(line, 5, -1, 1);
		if (strIsEmpty(desktopFile->exec) && strnIsEqualC(line, "Exec=", 5))
			desktopFile->exec = newStrSlice(line, 5, -1, 1);
		line = strTok(f, line, "\n");
	}

	strFree(&f);

	if (strIsEmpty(desktopFile->name) || strIsEmpty(desktopFile->exec)) {
		strFree(&desktopFile->name);
		strFree(&desktopFile->exec);
		return Err;
	}
	return Ok;
}

void proccessDesktopFile(const char *fileName, map *indexMap)
{
	DesktopFile *desktopFile = malloc(sizeof(DesktopFile));

	if (parseDesktopFile(fileName, desktopFile) == Ok) {
		removeFieldCodes(&desktopFile->exec);
		mapInsert(indexMap, &desktopFile->name, &desktopFile->exec);
	} else {
		free(desktopFile);
	}
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
	map programsMap = newMap(cmpString);
	for (int i = 0; dirs[i] != NULL; i++) {
		char fullDirName[PATH_MAX];
		snprintf(fullDirName, PATH_MAX, "%s", dirs[i]);
		expandPath(fullDirName, PATH_MAX);
		processDirectory(fullDirName, &programsMap);
	}
	return programsMap;
}

Result excuteProgram(const map *programsMap, const strSlice *programName)
{
	const string *command = mapFind(programsMap, programName);

	if (command != NULL && fork() == 0) {
		strPrintln(*command);
		redirectFd(STDOUT_FILENO, "/dev/null");
		redirectFd(STDERR_FILENO, "/dev/null");
		return system(command->data);
	}
	return Err;
}

void printProgramName(const void *key, const void *data, void *arg)
{
	(void)data;
	const string *strKey = key;
	fprintf((FILE *)arg, "%s\n", strKey->data);
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
	string output = strGetLine(pipe[Read]);
	if (!strIsEmpty(output)) {
		excuteProgram(&programsMap, &output);
		strFree(&output);
	}

	fclose(pipe[Read]);
	mapFree(&programsMap, freeStr, freeStr);
}
