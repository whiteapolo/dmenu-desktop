// execlp("dmenu", "dmenu", "-i", "-fn", "IosevkaTermNerdFontMono:size=15", NULL);
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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

#define BUF_SIZE 256

typedef struct {
	char name[BUF_SIZE];
	char exec[BUF_SIZE];
} DesktopEntry;

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

PATH_ERROR parseDesktopEntry(const char *fileName, DesktopEntry *entry)
{
	char line[256];
	bool doneName = false;
	bool doneExec = false;

	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		return PATH_FILE_NOT_FOUND;

	while (fgets(line, sizeof(line), fp) && (!doneName || !doneExec)) {

		strpoplast(line); // remove new line

		if (!doneName && !strncmp(line, "Name=", 5)) {
			strncpy(entry->name, line + 5, BUF_SIZE);
			doneName = true;
		}

		if (!doneExec && !strncmp(line, "Exec=", 5)) {
			strncpy(entry->exec, line + 5, BUF_SIZE);
			doneExec = true;
		}
	}

	fclose(fp);
	if (doneExec && doneName)
		return PATH_OK;
	return "could not parse desktopFile";
}

void printAppNameAndIndex(const char *fileName, FILE *output, map *indexMap)
{
	DesktopEntry entry;

	if (parseDesktopEntry(fileName, &entry) == PATH_OK) {
		fprintf(output, "%s\n", entry.name);
		char *key = strndup(entry.name, BUF_SIZE);
		char *data = strndup(entry.exec, BUF_SIZE);
		removeFieldCodes(data);
		mapInsert(indexMap, key, data);
	}
}

void processDirectory(const char *dirPath, FILE *output, map *indexMap)
{
	DIR *dir = opendir(dirPath);
	if (dir == NULL)
		return;

	const char *fileName;
	while ((fileName = nextInDir(dir)) != NULL) {
		char fullFileName[PATH_MAX];
		snprintf(fullFileName, sizeof(fullFileName), "%s/%s", dirPath, fileName);
		fullFileName[PATH_MAX - 1] = '\0';

		if (isExtentionEqual(fullFileName, "desktop"))
			printAppNameAndIndex(fullFileName, output, indexMap);
	}

	closedir(dir);
}

void die(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void initPipe(int pipedes[2])
{
	if (pipe(pipedes) == -1)
		die("pipe failed");
}

int fork1()
{
	const int pid = fork();
	if (pid == -1)
		die("fork failed");
	return pid;
}

int execCommandByName(const map execMap, const char *name)
{
	const char *command = (const char *)mapFind(execMap, name);

	if (command != NULL) {
		redirectFp(STDOUT_FILENO, "/dev/null");
		redirectFp(STDERR_FILENO, "/dev/null");
		return system(command);
	}
	return -1;
}

void excuteDmenu(const int argc, char **argv)
{
	char *args[argc + 2];
	args[0] = "dmenu";
	for (int i = 1; i < argc + 1; i++)
		args[i] = argv[i];
	args[argc + 1] = NULL;
	execvp("dmenu", args);
	die("execlp");
}

void runDmenuChildProcess(int inputPipe[2], int outputPipe[2], const int argc, char **argv)
{
	dup2(inputPipe[0], STDIN_FILENO);
	dup2(outputPipe[1], STDOUT_FILENO);

	close(inputPipe[1]);
	close(outputPipe[0]);

	excuteDmenu(argc, argv); // foward args to dmenu
}

bool getDmenuOutput(char *output, const int maxLen, const int outputFd)
{
	int len;
	if ((len = read(outputFd, output, maxLen)) > 0) {
		output[len] = '\0';
		strpoplast(output);
		return true;
	}
	return false;
}

void processDirectorys(const char *dirs[], FILE *output, map *indexMap)
{
	while (*dirs != NULL) {
		char fullDirName[PATH_MAX];
		snprintf(fullDirName, PATH_MAX, "%s", *dirs);
		expandPath(fullDirName, PATH_MAX);
		processDirectory(fullDirName, output, indexMap);
		dirs++;
	}
}

void processDmenuSelection(int inputPipe[2], int outputPipe[2], map *execMap)
{
	close(inputPipe[0]);
	close(outputPipe[1]);

	FILE *dmenuFp = fdopen(inputPipe[1], "w");
	if (dmenuFp == NULL)
		die("fdopen dmenu");

	processDirectorys(desktopAppsPath, dmenuFp, execMap);
	fclose(dmenuFp);

	char output[BUF_SIZE];

	if (getDmenuOutput(output, BUF_SIZE, outputPipe[0]))
		execCommandByName(*execMap, output);
	else
		printf("No selection made or dmenu closed.\n");

	close(outputPipe[0]);
}

int main(int argc, char **argv)
{
	int inputPipe[2];
	int outputPipe[2];
	map execMap = newMap((cmpKeysType)strcmp);

	initPipe(inputPipe);
	initPipe(outputPipe);

	if (fork1())
		runDmenuChildProcess(inputPipe, outputPipe, argc, argv);
	else
		processDmenuSelection(inputPipe, outputPipe, &execMap);

	mapFree(execMap, free, free);
}
