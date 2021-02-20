/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#define bzero(s, n) memset((s), 0, (n))
#endif

#include <plinkio/utarray.h>
#include <plinkio/fam.h>
#include <plinkio/fam_parse.h>
#include <plinkio/status.h>

/**
 * Creates mock versions of IO functions to allow unit testing.
 */
#ifdef UNIT_TESTING
    extern FILE *mock_fopen(const char *path, const char *mode);
    int mock_fclose(FILE *fp);

    #define fopen mock_fopen
    #define fclose mock_fclose
#endif

/**
 * Sample destructor. Ensures that the allocated
 * strings are freed properly.
 *
 * @param element Pointer to a sample.
 */
static void
utarray_sample_dtor(void *element)
{
    struct pio_sample_t *sample = (struct pio_sample_t *) element;

    if( sample->fid != NULL )
    {
        free( sample->fid );
    }
    if( sample->iid != NULL )
    {
        free( sample->iid );
    }
    if( sample->father_iid != NULL )
    {
        free( sample->father_iid );
    }
    if( sample->mother_iid != NULL )
    {
        free( sample->mother_iid );
    }
}

/**
 * Properties of the sample array for dtarray.
 */
UT_icd SAMPLE_ICD = {
    sizeof( struct pio_sample_t ),
    NULL,
    NULL,
    utarray_sample_dtor
};

pio_status_t
fam_open(struct pio_fam_file_t *fam_file, const char *path)
{
    pio_status_t status;
    FILE *fam_fp;

    bzero( fam_file, sizeof( *fam_file ) );
    fam_fp = fopen( path, "r" );
    if( fam_fp == NULL )
    {
        return PIO_ERROR;
    }

    fam_file->fp = fam_fp;
    utarray_new( fam_file->sample, &SAMPLE_ICD );
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

    bzero( fam_file, sizeof( *fam_file ) );
    fam_fp = fopen( path, "w" );
    if( fam_fp == NULL )
    {
        return PIO_ERROR;
    }

    fam_file->fp = fam_fp;

    utarray_new( fam_file->sample, &SAMPLE_ICD );
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
