#include <string.h>

#include "mock.h"
#include "private/utility.h"

/**
 * Returns the smallest value of x and y
 * compared with the < operator.
 */
#define MIN(x,y) ( ( (x) < (y) ) ? (x) : (y) )

/**
 * File string.
 */
static const char *g_file_str;

/**
 * Start index of next mock_fread call.
 */
static size_t g_file_pos = 0;

void
mock_init(const char *string)
{
    g_file_str = string;
    g_file_pos = 0;   
}

FILE *
mock_fopen(const char *path, const char *mode)
{
    UNUSED_PARAM(path);
    UNUSED_PARAM(mode);
    return stdin;
}

int
mock_fclose(FILE *fp)
{
    UNUSED_PARAM(fp);
    return 0;
}

int
mock_feof(FILE *stream)
{
    UNUSED_PARAM(stream);
    return g_file_pos >= strlen( g_file_str );
}

size_t
mock_fread(void *p, size_t size, size_t nmemb, FILE *stream)
{
    UNUSED_PARAM(stream);
    size_t length_left = strlen( g_file_str ) - g_file_pos;
    size_t bytes_to_copy = MIN( size * nmemb, length_left );
    g_file_pos += bytes_to_copy;

    if( bytes_to_copy > 0 )
    {
        strncpy( p, g_file_str, bytes_to_copy );
        return bytes_to_copy;
    }
    else
    {
        return 0;
    }
}
