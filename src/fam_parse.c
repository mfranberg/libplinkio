/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include "private/utility.h"
#include "private/plink_txt_parse.h"

#include <plinkio/utarray.h>
#include <plinkio/fam_parse.h>

/**
 * Creates mock versions of IO functions to allow unit testing.
 */
#ifdef UNIT_TESTING
    extern size_t mock_fread(void *p, size_t size, size_t nmemb, FILE *stream);
    extern int mock_feof(FILE *stream);

    #define fread mock_fread
    #define feof mock_feof
#endif

/**
 * Buffer size for reading CSV file.
 */
#define LIBPLINKIO_FAM_PARSE_BUFFER_SIZE_ 4096

struct fam_state_t
{
    /**
     * The next field to parse.
     */
    int field;

    /**
     * Determines whether there has been any error
     * during parsing.
     */
    int any_error;

    /**
     * Data of the sample that we are currently
     * parsing.
     */
    struct pio_sample_t cur_sample;

    /**
     * List of samples parsed so far.
     */
    UT_array *samples;
};

/**
 * Function that is called each time a new csv
 * field has been found. It is responsible for
 * determining which field is being parsed, and
 * updating the cur_sample object.
 *
 * @param field Csv field.
 * @param field_length Length of the field.
 * @param data A fam_state_t struct.
 */
static void
fam_new_field(char *field, size_t field_length, size_t field_num, void *data)
{
    UNUSED_PARAM(field_num);
    struct fam_state_t *state = (struct fam_state_t *) data;
    pio_status_t status;
    char *buffer;

    if( state->field == -1 )
    {
        return;
    }

    /* Null terminated field. */
    buffer = (char * ) malloc( sizeof( char ) * ( field_length + 1 ) );
    strncpy( buffer, field, field_length );
    buffer[ field_length ] = '\0';

    switch( state->field )
    {
        case 0:
            state->cur_sample.fid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 1:
            state->cur_sample.iid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 2:
            state->cur_sample.father_iid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 3:
            state->cur_sample.mother_iid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 4:
            state->cur_sample.sex = libplinkio_parse_sex_( buffer, field_length, &status );
            break;
        case 5:
            libplinkio_parse_phenotype_( buffer, field_length, &state->cur_sample, &status );
            break;
        default:
            status = PIO_ERROR;
            break;
    }

    free( buffer );

    if( status == PIO_OK )
    {
        state->field++;
    }
    else
    {
        state->any_error = 1;
        state->field = -1;
    }
}

/**
 * Function that is called each time a new row
 * has been found.
 *
 * @param number The row number.
 * @param data A fam_state_t struct.
 */
static void
fam_new_row(size_t number, void *data)
{
    UNUSED_PARAM(number);
    struct fam_state_t *state = (struct fam_state_t *) data;

    if( state->field == 6 )
    {
        state->cur_sample.pio_id = utarray_len( state->samples );
        utarray_push_back( state->samples, &state->cur_sample );
    }
    state->field = 0;
}

pio_status_t
parse_samples(FILE *fam_fp, UT_array *sample)
{
    char read_buffer[ LIBPLINKIO_FAM_PARSE_BUFFER_SIZE_ ];
    struct fam_state_t state = { 0 };
    libplinkio_txt_parser_private_t parser = { 0 };

    state.samples = sample;

    libplinkio_txt_parser_init_( &parser );
    do {
        size_t bytes_read = fread( read_buffer, sizeof( char ), LIBPLINKIO_FAM_PARSE_BUFFER_SIZE_ - 1, fam_fp );
        if (ferror(fam_fp)) goto error;
        read_buffer[bytes_read] = '\0';
        libplinkio_txt_parse_( &parser, read_buffer, bytes_read, &fam_new_field, &fam_new_row, (void *) &state );
    } while( !feof( fam_fp ) );

    libplinkio_txt_parse_fini_( &parser, &fam_new_field, &fam_new_row, (void *) &state );
    libplinkio_txt_parser_free_( &parser );
    
    if ( state.any_error != 0 ) goto error;
    return PIO_OK;

error:
    return PIO_ERROR;
}


pio_status_t
write_sample(FILE *fam_fp, struct pio_sample_t *sample)
{
    int sex = 0;
    int bytes_written = 0;
    if( sample->sex == PIO_MALE )
    {
        sex = 1;
    }
    else if( sample->sex == PIO_FEMALE )
    {
        sex = 2;
    }

    if( sample->affection == PIO_CONTINUOUS )
    {
        bytes_written = fprintf( fam_fp,
                 "%s\t%s\t%s\t%s\t%d\t%f\n", 
                 sample->fid,
                 sample->iid,
                 sample->father_iid,
                 sample->mother_iid,
                 sex,
                 sample->phenotype );
    }
    else
    {
        int affection = 0;
        if( sample->affection == PIO_CONTROL )
        {
            affection = 1;
        }
        else if( sample->affection == PIO_CASE )
        {
            affection = 2;
        }

        bytes_written = fprintf( fam_fp,
                 "%s\t%s\t%s\t%s\t%d\t%d\n", 
                 sample->fid,
                 sample->iid,
                 sample->father_iid,
                 sample->mother_iid,
                 sex,
                 affection );
    }

    if( bytes_written > 0 )
    {
        return PIO_OK;
    }
    else
    {
        return PIO_ERROR;
    }
}
