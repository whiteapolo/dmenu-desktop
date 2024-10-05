#ifndef PATH_H
#define PATH_H

#include <stdarg.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>

const int OK = 0;
const int ERR = -1;

const int READ = 0;
const int WRITE = 1;

int pathGetFmtSize(const char *fmt, ...);
int pathGetFmtSizeVa(const char *fmt, va_list ap);
int getFileSize(FILE *fp);

const char *getPathExtention(const char *path);
const char *getHomePath();
void expandPath(char *path, const int maxLen);
void compressPath(char *path);
bool isExtentionEqual(const char *path, const char *extention);

int dirTraverse(const char *dir, bool (*action)(const char *));
int traverseFile(const char *fileName, const int bufSize, bool (*action)(char[bufSize]));

bool isdir(const char *path);
bool isregfile(const char *fileName);
bool isfile(const char *fileName);

int echoFileWrite(const char *fileName, const char *fmt, ...);
int echoFileAppend(const char *fileName, const char *fmt, ...);
int readFile(const char *fileName, const char *fmt, ...);
int redirectFp(int srcFd, const char *destFileName);
int popen2(char *path, char *argv[], FILE *ppipe[2]);

void getFullFileName(const char *dirName, const char *fileName, char *dest, int destLen);

int nextInDir(DIR *dir, const char *dirName, char *destFileName, int destLen);

#ifdef PATH_IMPL

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int pathGetFmtSize(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const int size = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);
	return size;
}

int pathGetFmtSizeVa(const char *fmt, va_list ap)
{
	return vsnprintf(NULL, 0, fmt, ap);
}

int getFileSize(FILE *fp)
{
	int curr = ftell(fp);
	fseek(fp, 0, SEEK_END);
	const int size = ftell(fp);
	fseek(fp, curr, SEEK_SET);
	return size;
}

const char *getPathExtention(const char *path)
{
	const char *lastDot = strrchr(path, '.');
	if (lastDot == NULL)
		return path;
	return lastDot + 1;
}

const char *getHomePath()
{
	const char *home = getenv("HOME");
	if (home == NULL)
		return ".";
	return home;
}

void expandPath(char *path, const int maxLen)
{
	if (path[0] == '~') {
		char buf[maxLen];
		snprintf(buf, maxLen, "%s%s", getHomePath(), path + 1);
		strncpy(path, buf, maxLen);
	}
}

void compressPath(char *path)
{
	const char *home = getHomePath();
	const int homeLen = strlen(home);
	if (strncmp(home, path, homeLen) == 0) {
		const int bufLen = strlen(path) + homeLen;
		char buf[bufLen];
		snprintf(buf, bufLen, "~%s", path + homeLen);
		strncpy(path, buf, bufLen);
	}
}

int nextInDir(DIR *dir, const char *dirName, char *destFileName, int destLen)
{
	struct dirent *de;
	de = readdir(dir);
	if (de == NULL)
		return ERR;
	snprintf(destFileName, destLen, "%s/%s", dirName, de->d_name);
	return OK;
}

int dirTraverse(const char *dir, bool (*action)(const char *))
{
	struct dirent *de;
	DIR *dr = opendir(dir);

	if (dr == NULL)
		return ERR;

	while ((de = readdir(dr)) != NULL) {
		const char *file = de->d_name;
		char fullPath[PATH_MAX];
		const int len = snprintf(fullPath, PATH_MAX, "%s/%s", dir, file);
		fullPath[len] = '\0';
		if (action(fullPath) == false)
			break;
	}

	closedir(dr);
	return OK;
}

bool isExtentionEqual(const char *path, const char *extention)
{
	return strcmp(getPathExtention(path), extention) == 0;
}

bool isdir(const char *path)
{
	struct stat sb;
	stat(path, &sb);
	return S_ISDIR(sb.st_mode);
}

bool isregfile(const char *fileName)
{
	struct stat sb;
	stat(fileName, &sb);
	return S_ISREG(sb.st_mode);
}

bool isfile(const char *fileName)
{
	return !access(fileName, F_OK);
}

int echoFileWrite(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "w");

	if (fp == NULL)
		return ERR;

	vfprintf(fp, fmt, ap);
	va_end(ap);
	return OK;
}

int echoFileAppend(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "a");

	if (fp == NULL)
		return ERR;

	vfprintf(fp, fmt, ap);
	va_end(ap);
	return OK;
}

int readFile(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		return ERR;

	if (vfscanf(fp, fmt, ap) == EOF) {
        fclose(fp);
		return ERR;
    }

	va_end(ap);
    fclose(fp);
	return OK;
}

int redirectFp(int srcFd, const char *destFileName)
{
	int destFd = open(destFileName, O_WRONLY);
	if (destFd == -1)
		return ERR;

	if (dup2(destFd, srcFd) == -1) {
		close(destFd);
		return ERR;
	}

	close(destFd);
	return OK;
}

int traverseFile(const char *fileName, const int bufSize, bool (*action)(char[bufSize]))
{
	FILE *fp = fopen(fileName, "r");
	if (fp == NULL)
		return ERR;

	char buf[bufSize];
	while (fgets(buf, bufSize, fp) && action(buf));

	return OK;
}

void getFullFileName(const char *dirName, const char *fileName, char *dest, int destLen)
{
	snprintf(dest, destLen, "%s/%s", dirName, fileName);
}

int popen2(char *path, char *argv[], FILE *ppipe[2])
{
	int output[2];
	int input[2];

	if (pipe(output) == -1 || pipe(input) == -1)
		return ERR;

	int pid = fork();

	if (pid == -1)
		return ERR;

	if (pid) {
		// parent
		close(output[WRITE]);
		ppipe[WRITE] = fdopen(input[WRITE], "w");
		ppipe[READ] = fdopen(output[READ], "r");
	} else {
		// child
		dup2(input[READ], STDIN_FILENO);
		dup2(output[WRITE], STDOUT_FILENO);
		close(input[WRITE]);
		close(input[READ]);
		close(output[WRITE]);
		close(output[READ]);
		execvp(path, argv);
		exit(EXIT_FAILURE);
	}

	return OK;
}

#endif

#endif
