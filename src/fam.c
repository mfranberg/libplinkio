/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>

#include <plinkio/utarray.h>
#include <plinkio/fam.h>
#include <plinkio/fam_parse.h>
#include <plinkio/status.h>

#include "private/fam.h"
#include "private/sample.h"

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
fam_open(struct pio_fam_file_t *fam_file, const char *path)
{
    pio_status_t status;
    FILE *fam_fp;

    memset( fam_file, 0, sizeof( *fam_file ) );
    fam_fp = fopen( path, "r" );
    if( fam_fp == NULL )
    {
        return PIO_ERROR;
    }

    fam_file->fp = fam_fp;
    utarray_new( fam_file->sample, &LIBPLINKIO_SAMPLE_ICD_ );
    status = parse_samples( fam_file->fp, fam_file->sample );
    
    fclose( fam_fp );
    fam_file->fp = NULL;

    return status;
}

pio_status_t
fam_create(struct pio_fam_file_t *fam_file, const char *path, struct pio_sample_t *samples, size_t num_samples)
{
    size_t i;
    FILE *fam_fp;
    struct pio_sample_t sample_copy;

    memset( fam_file, 0, sizeof( *fam_file ) );
    fam_fp = fopen( path, "w" );
    if( fam_fp == NULL )
    {
        return PIO_ERROR;
    }

    fam_file->fp = fam_fp;

    utarray_new( fam_file->sample, &LIBPLINKIO_SAMPLE_ICD_ );
    for(i = 0; i < num_samples; i++)
    {
        if( write_sample( fam_fp, &samples[ i ] ) != PIO_OK )
        {
            return PIO_ERROR;
        }

        sample_copy.pio_id = i;
        sample_copy.fid = strdup( samples[ i ].fid );
        sample_copy.iid = strdup( samples[ i ].iid );
        sample_copy.mother_iid = strdup( samples[ i ].mother_iid );
        sample_copy.father_iid = strdup( samples[ i ].father_iid );
        sample_copy.sex = samples[ i ].sex;
        sample_copy.affection = samples[ i ].affection;
        sample_copy.phenotype = samples[ i ].phenotype;

        utarray_push_back( fam_file->sample, &sample_copy );
    }

    return PIO_OK;
}

struct pio_sample_t *
fam_get_sample(struct pio_fam_file_t *fam_file, size_t pio_id)
{
    return (struct pio_sample_t *) utarray_eltptr( fam_file->sample, pio_id );
}

size_t
fam_num_samples(struct pio_fam_file_t *fam_file)
{
    return utarray_len( fam_file->sample );
}

void
fam_close(struct pio_fam_file_t *fam_file)
{
    if( fam_file->sample == NULL )
    {
        return;
    }
    if( fam_file->fp != NULL )
    {
        fclose( fam_file->fp );
    }

    utarray_free( fam_file->sample );

    fam_file->sample = NULL;
    fam_file->fp = NULL;
}

pio_status_t libplinkio_fam_link_samples_to_file_(libplinkio_samples_private_t samples, struct pio_fam_file_t* fam_file, const char* fam_path, _Bool is_tmp) {
    FILE* fam_fp = NULL;
    if (!is_tmp) {
        fam_fp = fopen( fam_path, "w" );
        if( fam_fp == NULL )
        {
            return PIO_ERROR;
        }

        for ( struct pio_sample_t* sample = libplinkio_get_front_sample_(samples); sample != NULL; sample = libplinkio_get_next_sample_(samples, sample) ) {
            if( write_sample( fam_fp, sample ) != PIO_OK ) goto error;
        }
    }

    fam_file->sample = samples.ptr;
    fam_file->fp = fam_fp;
    return PIO_OK;

error:
    if (fam_fp != NULL) fclose(fam_fp);
    return PIO_ERROR;
}
