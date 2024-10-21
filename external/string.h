#ifndef STRING_H
#define STRING_H
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
	char *data;
	int len;
} string;

typedef string strSlice;

typedef struct {
	FILE *fp;
	string line;
	size_t lineCapacity;
} Scanner;

string newStr(const char *fmt, ...);
strSlice newStrSlice(const char *s, int len);
string newStrVa(const char *fmt, va_list ap);
string newStrFromExisting(const string *s);
string newStrFromArray(char *s[], const int len, const char *delim);

int getFmtSize(const char *fmt, ...);
int getFmtSizeVa(const char *fmt, va_list ap);

void strPushc(string *s, char c);
char strTopc(const strSlice *s);
char strPopc(string *s);
void strPushs(string *s, const char *fmt, ...);
void strPush(string *s, const strSlice *src);

bool strIsEmpty(const strSlice *s);
int strCmp(const strSlice *s1, const char *s2);
int strnCmp(const strSlice *s1, const char *s2, int n);
bool strIsEqual(const strSlice *s1, const char *s2);
bool strnIsEqual(const strSlice *s1, const char *s2, int n);

strSlice strTokStart(const strSlice *s, const char *delim);
strSlice strTok(const strSlice *s, const strSlice *prevSlice, const char *delim);
void strForEachTok(const strSlice *s, const char *delim, void (*action)(const strSlice *));
int strCountc(const strSlice *haystack, char needle);
int strCounts(const strSlice *haystack, const char *needle);
int strStr(const strSlice *haystack, const char *needle, int startOffset);
void strReplace(string *s, const char *sub, const char *by);
void strReplaceN(string *s, const char *sub, const char *by, int maxOccurrences);
void strReverse(string *s);
void strToLower(string *s);
void strToUpper(string *s);
int strToNumeric(const strSlice *s);
bool strIsNumeric(const strSlice *s);
void strTrim(string *s);
int strDisplayedLength(const string *s);
int strScanf(const strSlice *s, const char *fmt, ...);

void strFree(string *s);
void strPrint(const strSlice *s);
void strPrintln(const strSlice *s);
string input(const char *prompt);

Scanner newScanner(FILE *fp);
const string *scannerNextLine(Scanner *scanner);
void scannerFree(Scanner *scanner);

#ifdef STRING_IMPL
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

string newStr(const char *fmt, ...)
{
	// this is a comment
	va_list ap;
	va_start(ap, fmt);
	string s = newStrVa(fmt, ap);
	va_end(ap);
	return s;
}

string newStrVa(const char *fmt, va_list ap)
{
	string s;
	va_list ap1;
	va_copy(ap1, ap);

	s.len = getFmtSizeVa(fmt, ap1);
	va_end(ap1);

	s.data = malloc(sizeof(char) * s.len + 1);
	va_copy(ap1, ap);
	vsnprintf(s.data, s.len + 1, fmt, ap1);
	va_end(ap1);
    return s;
}

string newStrFromExisting(const strSlice *s)
{
	return newStr("%.*s", s->len, s->data);
}

string newStrFromArray(char *s[], const int len, const char *delim)
{
	string str = newStr("%s", *s);
	for (int i = 1; i < len; i++)
		strPushs(&str, "%s%s", delim, s[i]);
	return str;
}

strSlice newStrSlice(const char *s, int len)
{
	strSlice slice;
	slice.data = (char*)s;
	slice.len = len;
	return slice;
}

strSlice newStrSliceRange(const strSlice *s, int start, int end)
{
	strSlice slice;
	slice.data = s->data + start;
	slice.len = end - start;
	return slice;
}

int getFmtSize(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int size = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);
	return size;
}

int getFmtSizeVa(const char *fmt, va_list ap)
{
	return vsnprintf(NULL, 0, fmt, ap);
}

void strPushc(string *s, char c)
{
	s->data = realloc(s->data, sizeof(char) * (++s->len + 1));
	s->data[s->len - 1] = c;
	s->data[s->len] = '\0';
}

char strTopc(const strSlice *s)
{
	return s->data[s->len - 1];
}

char strPopc(string *s)
{
	char c = strTopc(s);
	s->data = realloc(s->data, sizeof(char) * (--s->len + 1));
	s->data[s->len - 1] = '\0';
	return c;
}

void strPushs(string *s, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	string tmp = newStrVa(fmt, ap);
	va_end(ap);

	for (int i = 0; i < tmp.len; i++)
		strPushc(s, tmp.data[i]);

	strFree(&tmp);
}

void strPush(string *s, const strSlice *src)
{
	strPushs(s, "%.*s", src->len, src->data);
}

bool strIsEmpty(const strSlice *s)
{
	return s->len == 0;
}

int strnCmp(const strSlice *s1, const char *s2, int n)
{
	return strncmp(s1->data, s2, n);
}

int strCmp(const strSlice *s1, const char *s2)
{
	return strnCmp(s1, s2, s1->len);
}

bool strIsEqual(const strSlice *s1, const char *s2)
{
	return strCmp(s1, s2) == 0;
}

bool strnIsEqual(const strSlice *s1, const char *s2, int n)
{
	return strnCmp(s1, s2, n) == 0;
}

strSlice strTokStart(const strSlice *s, const char *delim)
{
	const strSlice slice = newStrSlice(s->data, 0);
	return strTok(s, &slice, delim);
}

strSlice strTok(const strSlice *s, const strSlice *prevSlice, const char *delim)
{
	const char *start = prevSlice->data + prevSlice->len;
	const char *endptr = s->data + s->len;
	int len = 0;

	while (start < endptr && strchr(delim, *start) != NULL)
		start++;

	while (start + len < endptr && strchr(delim, start[len]) == NULL)
		len++;

	return newStrSlice(start, len);
}

void strForEachTok(const strSlice *s, const char *delim, void (*action)(const strSlice *))
{
	strSlice slice = strTokStart(s, delim);
	while (!strIsEmpty(&slice)) {
		action(&slice);
		slice = strTok(s, &slice, delim);
	}
}

int strCountc(const strSlice *haystack, char needle)
{
	int count = 0;
	for (int i = 0; i < haystack->len; i++)
		if (haystack->data[i] == needle)
			count++;
	return count;
}

int strCounts(const strSlice *haystack, const char *needle)
{
	int count = 0;
	int i = -1;

	while ((i = strStr(haystack, needle, i + 1)) != -1)
		count++;

	return count;
}

int strStr(const strSlice *haystack, const char *needle, int startOffset)
{
	const int needleLen = strlen(needle);
	for (int i = startOffset; i < haystack->len - needleLen + 1; ++i) {
		const strSlice slice = newStrSlice(haystack->data + i, needleLen);
		if (strIsEqual(&slice, needle))
			return i;
	}

	return -1;
}

void strReplace(string *s, const char *sub, const char *by)
{
	strReplaceN(s, sub, by, s->len);
}

void strReplaceN(string *s, const char *sub, const char *by, int maxOccurrences)
{
	string result = newStr("");
	int i = 0;
	const int sublen = strlen(sub);

	while (i < s->len - sublen + 1 && maxOccurrences > 0) {
		strSlice tmp = newStrSliceRange(s, i, i + sublen);
		if (strIsEqual(&tmp, sub)) {
			strPushs(&result, "%s", by);
			i += sublen;
			maxOccurrences--;
		} else {
			strPushc(&result, s->data[i++]);
		}
	}

	// push the rest of the string
	strPushs(&result, "%.*s", s->len - i, s->data + i);

	strFree(s);
	*s = result;
}

void strReverse(string *s)
{
	string tmp = newStr("");

	for (int i = s->len - 1; i >= 0; --i)
		strPushc(&tmp, s->data[i]);

	strFree(s);
	*s = tmp;
}

void strToLower(string *s)
{
	for (int i = 0; i < s->len; ++i)
		s->data[i] = tolower(s->data[i]);
}

void strToUpper(string *s)
{
	for (int i = 0; i < s->len; ++i)
		s->data[i] = toupper(s->data[i]);
}

int strToNumeric(const strSlice *s)
{
	int num = 0;
	for (int i = 0; i < s->len; i++) {
		num *= 10;
		num += s->data[i];
	}
	return num;
}

bool strIsNumeric(const strSlice *s)
{
	for (int i = 0; i < s->len; ++i)
		if (!isdigit(s->data[i]))
			return false;
	return true;
}

void strTrim(string *s)
{
	int start = 0;
	int end = s->len - 1;

	while (start < s->len && isspace(s->data[start]))
		start++;

	while (end > 0 && isspace(s->data[end]))
		end--;

	const int newLen = end - start + 1;
	memmove(s->data, s->data + start, newLen);
	s->len = newLen;
	s->data = realloc(s->data, sizeof(char) * newLen + 1);
	s->data[newLen] = '\0';
}

int strScanf(const strSlice *s, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = vsscanf(s->data, fmt, ap);
	va_end(ap);
	return ret;
}

void strFree(string *s)
{
	free(s->data);
}

void strPrint(const strSlice *s)
{
    printf("%.*s", s->len, s->data);
}

void strPrintln(const strSlice *s)
{
    printf("%.*s\n", s->len, s->data);
}

string input(const char *prompt)
{
	string s = newStr("");
	printf("%s", prompt);
	char c;
	while ((c = fgetc(stdin)) != '\n')
		strPushc(&s, c);
	return s;
}

Scanner newScanner(FILE *fp)
{
	Scanner scanner;
	scanner.fp = fp;
	scanner.line.data = NULL;
	scanner.line.len = 0;
	scanner.lineCapacity = 0;
	return scanner;
}

const string *scannerNextLine(Scanner *scanner)
{
	scanner->line.len = getline(&scanner->line.data, &scanner->lineCapacity, scanner->fp);
	if (scanner->line.len == -1)
		return NULL;
	if (scanner->line.len > 0 && scanner->line.data[scanner->line.len - 1] == '\n')
		scanner->line.data[scanner->line.len - 1] = '\0';
	return &scanner->line;
}

void scannerFree(Scanner *scanner)
{
	free(scanner->line.data);
}

#endif
#endif
