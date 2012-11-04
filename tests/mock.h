#ifndef __MOCK_H__
#define __MOCK_H__

#include <stdio.h>
#include <stdlib.h>

/**
 * Initializes the mock suite. Just call this function before you
 * are using functions that depend on fread etc.
 */
void mock_init(const char *string);

/**
 * Mocks fopen, always returns stdin.
 */
FILE *mock_fopen(const char *path, const char *mode);

/**
 * Mocks fclose, always returns 0.
 */
int mock_fclose(FILE *fp);

/**
 * Mocks feof, returns eof when we are at the end of the string.
 */
int mock_feof(FILE *stream);

/**
 * Mock fread, reads from a string instead of a file.
 */
size_t mock_fread(void *p, size_t size, size_t nmemb, FILE *stream);

#endif /* End of __MOCK_H__ */
