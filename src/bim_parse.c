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
#include <plinkio/bim.h>
#include <plinkio/bim_parse.h>

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
#define LIBPLINKIO_BIM_PARSE_BUFFER_SIZE_ 4096

struct bim_state_t
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
     * Data of the locus that we are currently
     * parsing.
     */
    struct pio_locus_t cur_locus;

    /**
     * List of loci parsed so far.
     */
    UT_array *locus;
};

/**
 * Function that is called each time a new csv
 * field has been found. It is responsible for
 * determining which field is being parsed, and
 * updating the cur_locus object.
 *
 * @param field Csv field.
 * @param field_length Length of the field.
 * @param data A bim_state_t struct.
 */
static void
bim_new_field(char *field, size_t field_length, size_t field_num, void *data)
{
    UNUSED_PARAM(field_num);
    struct bim_state_t *state = (struct bim_state_t *) data;
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
            state->cur_locus.chromosome = libplinkio_parse_chr_( buffer, field_length, &status );
            break;
        case 1:
            state->cur_locus.name = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 2:
            state->cur_locus.position = libplinkio_parse_genetic_position_( buffer, field_length, &status );
            break;
        case 3:
            state->cur_locus.bp_position = libplinkio_parse_bp_position_( buffer, field_length, &status );
            break;
        case 4:
            state->cur_locus.allele1 = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 5:
            state->cur_locus.allele2 = libplinkio_parse_str_( buffer, field_length, &status );
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
 * @param data A bim_state_t struct.
 */
static void
bim_new_row(size_t number, void *data)
{
    UNUSED_PARAM(number);
    struct bim_state_t *state = (struct bim_state_t *) data;

    if( state->field == 6 )
    {
        state->cur_locus.pio_id = utarray_len( state->locus );
        utarray_push_back( state->locus, &state->cur_locus );
    }
    state->field = 0;
}

pio_status_t
parse_loci(FILE *bim_fp, UT_array *locus)
{
    char read_buffer[ LIBPLINKIO_BIM_PARSE_BUFFER_SIZE_ ];
    struct bim_state_t state = { 0 };
    libplinkio_txt_parser_private_t parser = { 0 };

    state.locus = locus;

    libplinkio_txt_parser_init_( &parser );
    do {
        size_t bytes_read = fread( read_buffer, sizeof( char ), LIBPLINKIO_BIM_PARSE_BUFFER_SIZE_ - 1, bim_fp );
        if (ferror(bim_fp)) goto error;
        read_buffer[bytes_read] = '\0';
        libplinkio_txt_parse_( &parser, read_buffer, bytes_read, &bim_new_field, &bim_new_row, (void *) &state );
    } while( !feof( bim_fp ) );

    libplinkio_txt_parse_fini_( &parser, &bim_new_field, &bim_new_row, (void *) &state );
    libplinkio_txt_parser_free_( &parser );
    
    if ( state.any_error != 0 ) goto error;
    return PIO_OK;

error:
    return PIO_ERROR;
}

pio_status_t
write_locus(FILE *bim_fp, struct pio_locus_t *locus)
{
    int bytes_written = fprintf( bim_fp,
                    "%d\t%s\t%f\t%lld\t%s\t%s\n",
                    locus->chromosome,
                    locus->name,
                    locus->position,
                    locus->bp_position,
                    locus->allele1,
                    locus->allele2
                    );

    if( bytes_written > 0 )
    {
        return PIO_OK;
    }
    else
    {
        return PIO_ERROR;
    }
}
