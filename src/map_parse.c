/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include <plinkio/utarray.h>

#include "private/utility.h"
#include "private/plink_txt_parse.h"
#include "private/map_parse.h"

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
#define LIBPLINKIO_MAP_PARSE_BUFFER_SIZE_ 4096

struct map_state_t
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
     * Temporary buffer for optional field
     */
    char* tmp_buffer;
    size_t tmp_buffer_length;

    /**
     * List of loci parsed so far.
     */
    libplinkio_loci_private_t loci;
};

/**
 * Function that is called each time a new csv
 * field has been found. It is responsible for
 * determining which field is being parsed, and
 * updating the cur_locus object.
 *
 * @param field Csv field.
 * @param field_length Length of the field.
 * @param data A map_state_t struct.
 */
static void
map_new_field(char *field, size_t field_length, size_t field_num, void *data)
{
    UNUSED_PARAM(field_num);
    struct map_state_t *state = (struct map_state_t *) data;
    pio_status_t status = PIO_OK;
    char *buffer = NULL;

    if( state->field == -1 ) goto error;

    /* Null terminated field. */
    buffer = (char*)calloc((field_length + 1), sizeof(char));
    if (buffer == NULL) goto error;
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
            // This "if" is a workaroud for false positives of test_free in cmockery.
            if (state->tmp_buffer != NULL) free(state->tmp_buffer);
            state->tmp_buffer = buffer;
            state->tmp_buffer_length = field_length;
            buffer = NULL;
            status = PIO_OK;
            break;
        case 3:
            state->cur_locus.bp_position = libplinkio_parse_bp_position_( buffer, field_length, &status );
            break;
        default:
            status = PIO_ERROR;
            break;
    }
    // This "if" is a workaroud for false positives of test_free in cmockery.
    if (buffer != NULL) free( buffer );
    buffer = NULL;

    if( status == PIO_OK )
    {
        state->field++;
    }
    else
    {
        goto error;
    }
    return;

error:
    free(buffer);
    state->any_error = 1;
    state->field = -1;
    return;
}

/**
 * Function that is called each time a new row
 * has been found.
 *
 * @param number The row number.
 * @param data A map_state_t struct.
 */
static void
map_new_row(size_t number, void *data)
{
    UNUSED_PARAM(number);
    struct map_state_t *state = (struct map_state_t *) data;

    if( state->field == 3 ) {
        state->cur_locus.position = 0;
        pio_status_t status = PIO_OK;
        state->cur_locus.bp_position = libplinkio_parse_bp_position_( state->tmp_buffer, state->tmp_buffer_length, &status );
        if (status != PIO_OK) goto error;
    } else if( state->field == 4) {
        pio_status_t status = PIO_OK;
        state->cur_locus.position = libplinkio_parse_genetic_position_( state->tmp_buffer, state->tmp_buffer_length, &status );
        if (status != PIO_OK) goto error;
    }

    free( state->tmp_buffer );
    state->tmp_buffer = NULL;

    if (state->field == 3 || state->field == 4) {
        state->cur_locus.allele1 = NULL;
        state->cur_locus.allele2 = NULL;

        state->cur_locus.pio_id = libplinkio_get_num_loci_(state->loci);;
        libplinkio_add_locus_( state->loci, &state->cur_locus );
        state->cur_locus = (struct pio_locus_t){ 0 };
        state->field = 0;
    } else {
        goto error;
    }
    return;

error:
    state->any_error = 1;
    state->field = -1;
    state->cur_locus = (struct pio_locus_t){ 0 };
    return;
}

pio_status_t
libplinkio_map_parse_loci_(FILE *map_fp, libplinkio_loci_private_t loci)
{
    char read_buffer[ LIBPLINKIO_MAP_PARSE_BUFFER_SIZE_ ];
    struct map_state_t state = { 0 };
    libplinkio_txt_parser_private_t parser = { 0 };

    state.loci = loci;

    libplinkio_txt_parser_init_( &parser );
    do {
        size_t bytes_read = fread( read_buffer, sizeof( char ), LIBPLINKIO_MAP_PARSE_BUFFER_SIZE_ - 1, map_fp );
        if (ferror(map_fp)) goto error;
        read_buffer[bytes_read] = '\0';
        libplinkio_txt_parse_( &parser, read_buffer, bytes_read, &map_new_field, &map_new_row, (void *) &state );
    } while( !feof( map_fp ) );

    libplinkio_txt_parse_fini_( &parser, &map_new_field, &map_new_row, (void *) &state );
    libplinkio_txt_parser_free_( &parser );
    
    if ( state.any_error != 0 ) goto error;
    return PIO_OK;

error:
    return PIO_ERROR;
}

