/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>

#include <plinkio/utarray.h>
#include <plinkio/bim.h>
#include <plinkio/bim_parse.h>
#include <plinkio/status.h>

#include "private/bim.h"
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
bim_open(struct pio_bim_file_t *bim_file, const char *path)
{
    int status;
    FILE *bim_fp;
    memset( bim_file, 0, sizeof( *bim_file ) );
    bim_fp = fopen( path, "r" );
    if( bim_fp == NULL )
    {
        return PIO_ERROR;
    }

    bim_file->fp = bim_fp;
    utarray_new( bim_file->locus, &LIBPLINKIO_LOCUS_ICD_ );
    status = parse_loci( bim_file->fp, bim_file->locus );

    fclose( bim_fp );
    bim_file->fp = NULL;

    return status;
}

pio_status_t
bim_create(struct pio_bim_file_t *bim_file, const char *path)
{
    FILE *bim_fp;
    memset( bim_file, 0, sizeof( *bim_file ) );
    bim_fp = fopen( path, "w" );
    if( bim_fp == NULL )
    {
        return PIO_ERROR;
    }

    bim_file->fp = bim_fp;
    utarray_new( bim_file->locus, &LIBPLINKIO_LOCUS_ICD_ );

    return PIO_OK;
}

pio_status_t
bim_write(struct pio_bim_file_t *bim_file, struct pio_locus_t *locus)
{
    struct pio_locus_t locus_copy;
    if( write_locus( bim_file->fp, locus ) == PIO_OK )
    {
        locus_copy.pio_id = bim_num_loci( bim_file );
        locus_copy.chromosome = locus->chromosome;
        locus_copy.name = strdup( locus->name );
        locus_copy.position = locus->position;
        locus_copy.bp_position = locus->bp_position;
        locus_copy.allele1 = strdup( locus->allele1 );
        locus_copy.allele2 = strdup( locus->allele2 );

        utarray_push_back( bim_file->locus, &locus_copy );
        return PIO_OK;
    }
    else
    {
        return PIO_ERROR;
    }
}

struct pio_locus_t *
bim_get_locus(struct pio_bim_file_t *bim_file, size_t pio_id)
{
    return (struct pio_locus_t *) utarray_eltptr( bim_file->locus, pio_id );  
}

size_t
bim_num_loci(struct pio_bim_file_t *bim_file)
{
    return utarray_len( bim_file->locus );
}

void
bim_close(struct pio_bim_file_t *bim_file)
{
    if( bim_file->locus == NULL )
    {
        return;
    }
    if( bim_file->fp != NULL )
    {
        fclose( bim_file->fp );
    }

    utarray_free( bim_file->locus );
    bim_file->locus = NULL;
    bim_file->fp = NULL;
}

pio_status_t libplinkio_bim_link_loci_to_file_(libplinkio_loci_private_t loci, struct pio_bim_file_t* bim_file, const char* bim_path, _Bool is_tmp) {
    FILE* bim_fp = NULL;
    if (!is_tmp) {
        bim_fp = fopen( bim_path, "w" );
        if( bim_fp == NULL )
        {
            return PIO_ERROR;
        }

        for ( struct pio_locus_t* locus = libplinkio_get_front_locus_(loci); locus != NULL; locus = libplinkio_get_next_locus_(loci, locus) ) {
            if( write_locus( bim_fp, locus ) != PIO_OK ) goto error;
        }
    }

    bim_file->locus = loci.ptr;
    bim_file->fp = bim_fp;
    return PIO_OK;

error:
    if (bim_fp != NULL) fclose(bim_fp);
    return PIO_ERROR;
}
