/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include <csv.h>

#include <plinkio/utarray.h>
#include <plinkio/bim.h>
#include <plinkio/bim_parse.h>

#include <./plinkio_private.h>

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
#define BUFFER_SIZE 4096

struct state_t
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
 * Libcsv delimiter function, fields are delimited by
 * any amount of white space.
 *
 * @param c Character.
 *
 * @return True if c is white space, false otherwise.
 */
int
bim_is_delim(unsigned char c)
{
    return c == '\t' || c == ' ';
}

/**
 * Parses an string from a csv field.
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param status Status of the conversion.
 *
 * @return The parsed csv field, or NULL if it could
 *         not be parsed. Caller is responsible for 
 *         deallocating the memory.
 */
static char *
parse_str(const char *field, size_t length, pio_status_t *status)
{
    if( length > 0 )
    {
        char *iid = (char *) malloc( sizeof( char ) * ( length + 1 ) );
        strncpy( iid, field, length + 1 );

        *status = PIO_OK;
        return iid;
    }
    else
    {
        *status = PIO_ERROR;
        return NULL;
    }
}

/**
 * Parses a chromosome number and returns it.
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param status Status of the conversion.
 *
 * @return The parsed csv field, or 0 if it could
 *         not be parsed.
 */
static unsigned char
parse_chr(const char *field, size_t length, pio_status_t *status)
{
    char *endptr;
    unsigned char chr = (unsigned char) strtol( field, &endptr, 10 );
    if( length > 0 && ( endptr == NULL || *endptr == '\0' ) )
    {
        *status = PIO_OK;
        return chr;
    }

    *status = PIO_ERROR;
    return 0;
}

/**
 * Parses a genetic distance (float).
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param status Status of the conversion.
 *
 * @return The parsed csv field, or -1 if it could
 *         not be parsed.
 */
static float
parse_genetic_position(const char *field, size_t length, pio_status_t *status)
{
    char *endptr;
    float position = (float) strtod( field, &endptr );
    if( length > 0 && ( endptr == NULL || *endptr == '\0' ) )
    {
        *status = PIO_OK;
        return position;
    }

    *status = PIO_ERROR;
    return -1.0f;
}

/**
 * Parses a bp distance.
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param status Status of the conversion.
 *
 * @return The parsed csv field, or -1 if it could
 *         not be parsed.
 */
static long long
parse_bp_position(const char *field, size_t length, pio_status_t *status)
{
    char *endptr;
    long long int position = strtoll( field, &endptr, 10 );
    if( length > 0 && ( endptr == NULL || *endptr == '\0' ) )
    {
        *status = PIO_OK;
        return position;
    }

    *status = PIO_ERROR;
    return -1;
}

/**
 * Function that is called each time a new csv
 * field has been found. It is responsible for
 * determining which field is being parsed, and
 * updating the cur_locus object.
 *
 * @param field Csv field.
 * @param field_length Length of the field.
 * @param data A state_t struct.
 */
static void
new_field(void *field, size_t field_length, void *data)
{
    struct state_t *state = (struct state_t *) data;
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
            state->cur_locus.chromosome = parse_chr( buffer, field_length, &status );
            break;
        case 1:
            state->cur_locus.name = parse_str( buffer, field_length, &status );
            break;
        case 2:
            state->cur_locus.position = parse_genetic_position( buffer, field_length, &status );
            break;
        case 3:
            state->cur_locus.bp_position = parse_bp_position( buffer, field_length, &status );
            break;
        case 4:
            state->cur_locus.allele1 = parse_str( buffer, field_length, &status );
            break;
        case 5:
            state->cur_locus.allele2 = parse_str( buffer, field_length, &status );
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
 * @param data A state_t struct.
 */
static void
new_row(int number, void *data)
{
    UNUSED_PARAM(number);
    struct state_t *state = (struct state_t *) data;

    if( state->field != -1 )
    {
        state->cur_locus.pio_id = utarray_len( state->locus );
        utarray_push_back( state->locus, &state->cur_locus );
    }
    state->field = 0;
}

pio_status_t
parse_loci(FILE *bim_fp, UT_array *locus)
{
    char read_buffer[ BUFFER_SIZE ];
    struct state_t state = { 0 };
    struct csv_parser parser;

    state.locus = locus;

    csv_init( &parser, 0 );
    csv_set_delim_func( &parser, bim_is_delim );
    while( !feof( bim_fp ) )
    {
        int bytes_read = fread( &read_buffer[ 0 ], sizeof( char ), BUFFER_SIZE, bim_fp );
        csv_parse( &parser, read_buffer, bytes_read, &new_field, &new_row, (void *) &state );
    }

    csv_fini( &parser, new_field, new_row, (void *) &state );
    csv_free( &parser );
    
    return ( state.any_error == 0 ) ? PIO_OK : PIO_ERROR;
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
