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
		return -1;

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
		return 0;
	free(desktopFile->name);
	free(desktopFile->exec);
	errno = EINVAL;
	return -1;
}

void proccessDesktopFile(const char *fileName, FILE *output, map *indexMap)
{
	DesktopFile desktopFile;

	if (parseDesktopFile(fileName, &desktopFile) == 0) {
		fprintf(output, "%s\n", desktopFile.name);
		removeFieldCodes(desktopFile.exec);
		mapInsert(indexMap, desktopFile.name, desktopFile.exec);
	}

	// no need to free desktopFile because we send it to mapInsert
}

void processDirectory(const char *dirPath, FILE *output, map *indexMap)
{
	DIR *dir = opendir(dirPath);

	if (dir == NULL)
		return;

	char fileName[PATH_MAX];
	while (nextInDir(dir, dirPath, fileName, sizeof(fileName)) == OK)
		if (isExtentionEqual(fileName, "desktop"))
			proccessDesktopFile(fileName, output, indexMap);

	closedir(dir);
}

void die(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void pipe1(int pipedes[2])
{
	if (pipe(pipedes) == -1)
		die("pipe() failed");
}

int fork1()
{
	const int pid = fork();
	if (pid == -1)
		die("fork() failed");
	return pid;
}

int execCommandByName(const map execMap, const char *name)
{
	const char *command = (const char *)mapFind(execMap, name);

	if (command != NULL && fork() == 0) {
		redirectFp(STDOUT_FILENO, "/dev/null");
		redirectFp(STDERR_FILENO, "/dev/null");
		return system(command);
	}
	return ERR;
}

// void execInBackground(char *args[])
// {
// 	if (!fork())
// 		execvp(args[0], args);
// }

void runDmenuChildProcess(int inputPipe[2], int outputPipe[2], char **argv)
{
	dup2(inputPipe[0], STDIN_FILENO);
	dup2(outputPipe[1], STDOUT_FILENO);

	close(inputPipe[1]);
	close(outputPipe[0]);

	execvp(argv[0], argv);
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

void processDirectories(const char *dirs[], FILE *output, map *indexMap)
{
	while (*dirs != NULL) {
		char fullDirName[PATH_MAX];
		snprintf(fullDirName, PATH_MAX, "%s", *dirs);
		expandPath(fullDirName, PATH_MAX);
		processDirectory(fullDirName, output, indexMap);
		dirs++;
	}
}

int main(int, char **argv)
{
	FILE *pipe[2];
	map execMap = newMap((cmpKeysType)strcmp);
	argv[0] = "dmenu";

	if (popen2("dmenu", argv, pipe) == ERR)
		die("popen2()");

	processDirectories(desktopAppsPath, pipe[WRITE], &execMap);
	fclose(pipe[WRITE]);

	char *buf = NULL;
	size_t bufLen = 0;
	if (getline(&buf, &bufLen, pipe[READ]) > 0) {
		strpoplast(buf); // remove new line
		execCommandByName(execMap, buf);
	}

	free(buf);
	fclose(pipe[READ]);
	mapFree(execMap, free, free);
}
