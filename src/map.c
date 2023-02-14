/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>

#include "private/map.h"
#include "private/map_parse.h"
#include "private/locus.h"

/**
 * Creates mock versions of IO functions to allow unit testing.
 */
#ifdef UNIT_TESTING
    extern FILE *mock_fopen(const char *path, const char *mode);
    int mock_fclose(FILE *fp);

    #define fopen mock_fopen
    #define fclose mock_fclose
#endif

pio_status_t
libplinkio_map_open_(libplinkio_loci_private_t *loci, const char *path)
{
    FILE *map_fp = NULL;
    *loci = libplinkio_init_loci_();

    map_fp = fopen( path, "r" );
    if( map_fp == NULL ) goto error;

    *loci = libplinkio_new_loci_();
    if (libplinkio_map_parse_loci_( map_fp, *loci ) != PIO_OK) goto error;

    fclose( map_fp );

    return PIO_OK;

error:
    libplinkio_free_loci_(*loci);
    *loci = libplinkio_init_loci_();
    if (map_fp != NULL) fclose( map_fp );
    return PIO_ERROR;
}

