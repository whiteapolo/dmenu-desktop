#ifndef PATH_H
#define PATH_H

#include <stdarg.h>

typedef const char* PATH_ERROR;

void pathPrintError(const char *label, const PATH_ERROR e);

int pathGetFmtSize(const char *fmt, ...);
int pathGetFmtSizeVa(const char *fmt, va_list ap);
int getFileSize(FILE *fp);

const char *getPathExtention(const char *path);
const char *getHomePath();
void expandPath(char *path, const int maxLen);
void compressPath(char *path);
bool isExtentionEqual(const char *path, const char *extention);

PATH_ERROR dirTraverse(const char *dir, bool (*action)(const char *));
PATH_ERROR traverseFile(const char *fileName, const int bufSize, bool (*action)(char[bufSize]));

bool isdir(const char *path);
bool isregfile(const char *fileName);
bool isfile(const char *fileName);

PATH_ERROR echoFileWrite(const char *fileName, const char *fmt, ...);
PATH_ERROR echoFileAppend(const char *fileName, const char *fmt, ...);
PATH_ERROR readFile(const char *fileName, const char *fmt, ...);
PATH_ERROR redirectFp(int srcFd, const char *dest);

#ifdef PATH_IMPL

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const char *PATH_FILE_NOT_FOUND = "File not found";
const char *PATH_SCANF_ERROR = "scanf error";
const char *PATH_OK = NULL;

void pathPrintError(const char *label, const PATH_ERROR e)
{
	printf("%s:%s", label, e);
}

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

const char *nextInDir(DIR *dir)
{
	struct dirent *de;
	de = readdir(dir);
	if (de == NULL)
		return NULL;
	return de->d_name;
}

PATH_ERROR dirTraverse(const char *dir, bool (*action)(const char *))
{
	struct dirent *de;
	DIR *dr = opendir(dir);

	if (dr == NULL)
		return PATH_FILE_NOT_FOUND;

	while ((de = readdir(dr)) != NULL) {
		const char *file = de->d_name;
		char fullPath[PATH_MAX];
		const int len = snprintf(fullPath, PATH_MAX, "%s/%s", dir, file);
		fullPath[len] = '\0';
		if (action(fullPath) == false)
			break;
	}

	closedir(dr);
	return PATH_OK;
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

PATH_ERROR echoFileWrite(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "w");

	if (fp == NULL)
		return PATH_FILE_NOT_FOUND;

	vfprintf(fp, fmt, ap);
	va_end(ap);
	return PATH_OK;
}

PATH_ERROR echoFileAppend(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "a");

	if (fp == NULL)
		return PATH_FILE_NOT_FOUND;

	vfprintf(fp, fmt, ap);
	va_end(ap);
	return PATH_OK;
}

PATH_ERROR readFile(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		return PATH_FILE_NOT_FOUND;

	if (vfscanf(fp, fmt, ap) == EOF) {
        fclose(fp);
		return PATH_SCANF_ERROR;
    }

	va_end(ap);
    fclose(fp);
	return PATH_OK;
}

PATH_ERROR redirectFp(int srcFd, const char *dest)
{
	int destFd = open(dest, O_WRONLY);
	if (destFd == -1)
		return PATH_FILE_NOT_FOUND;

	if (dup2(destFd, srcFd) == -1)
		return "dup2 failed";

	close(destFd);
	return PATH_OK;
}

PATH_ERROR traverseFile(const char *fileName, const int bufSize, bool (*action)(char[bufSize]))
{
	FILE *fp = fopen(fileName, "r");
	if (fp == NULL)
		return PATH_FILE_NOT_FOUND;

	char buf[bufSize];
	while (fgets(buf, bufSize, fp) && action(buf));

	return PATH_OK;
}

#endif

#endif
