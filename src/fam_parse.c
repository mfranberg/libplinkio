#include <stdio.h>
#include <stdlib.h>

#include <csv.h>
#include <utarray.h>

#include <fam_parse.h>

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
 * Libcsv delimiter function, fields are delimited by
 * any amount of white space.
 *
 * @param c Character.
 *
 * @return True if c is white space, false otherwise.
 */
int
fam_is_delim(unsigned char c)
{
    return c == '\t' || c == ' ';
}

/**
 * Parses an iid from a csv field.
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
parse_iid(const char *field, size_t length, pio_status_t *status)
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
 * Parses sex from a csv field.
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param status Status of the conversion.
 *
 * @return The parsed csv field, or PIO_UNKOWN along with
 *         status = PIO_ERROR if it could not be parsed.
 */
static enum sex_t
parse_sex(const char *field, size_t length, pio_status_t *status)
{
    if( length != 1 )
    {
        *status = PIO_ERROR;
        return PIO_UNKOWN;
    }

    *status = PIO_OK;
    if( *field == '1' )
    {
        return PIO_MALE;
    }
    else if( *field == '2' )
    {
        return PIO_FEMALE;
    }
    else
    {
        return PIO_UNKOWN;
    }
}

/**
 * Parses a phenotype from a csv field.
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param sample The affection and phenotype will
 *               be updated.
 * @param status Status of the conversion.
 */
static void
parse_phenotype(const char *field, size_t length, struct pio_sample_t *sample, pio_status_t *status)
{
    char *endptr;
    double phenotype_float;

    if( length == 1 )
    {
        int valid = 0;
        switch( *field )
        {
            case '1':
                sample->affection = PIO_CONTROL;
                sample->phenotype = 0.0f;
                valid = 1;
                break;
            case '2':
                sample->affection = PIO_CASE;
                sample->phenotype = 1.0f;
                valid = 1;
                break;
            case '0':
                sample->affection = PIO_MISSING;
                sample->phenotype = -9.0f;
                valid = 1;
                break;
            default:
                break;
        }

        if( valid == 1 )
        {
            *status = PIO_OK;
            return;
        }
    }
    if( strncmp( field, "-9", length ) == 0 )
    {
        sample->affection = PIO_MISSING;
        *status = PIO_OK;

        return;
    }

    phenotype_float = strtod( field, &endptr );
    if( length > 0 && ( endptr == NULL || *endptr == '\0' ) )
    {
        sample->phenotype = (float) phenotype_float;
        sample->affection = PIO_CONTINUOUS;
        *status = PIO_OK;
        return;
    }

    *status = PIO_ERROR;
    return;
}

/**
 * Function that is called each time a new csv
 * field has been found. It is responsible for
 * determining which field is being parsed, and
 * updating the cur_sample object.
 *
 * @param field Csv field.
 * @param field_length Length of the field.
 * @param data A state_t struct.
 */
static void
new_field(void *field, unsigned long int field_length, void *data)
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
            state->cur_sample.fid = parse_iid( buffer, field_length, &status );
            break;
        case 1:
            state->cur_sample.iid = parse_iid( buffer, field_length, &status );
            break;
        case 2:
            state->cur_sample.father_iid = parse_iid( buffer, field_length, &status );
            break;
        case 3:
            state->cur_sample.mother_iid = parse_iid( buffer, field_length, &status );
            break;
        case 4:
            state->cur_sample.sex = parse_sex( buffer, field_length, &status );
            break;
        case 5:
            parse_phenotype( buffer, field_length, &state->cur_sample, &status );
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
    struct state_t *state = (struct state_t *) data;

    if( state->field != -1 )
    {
        state->cur_sample.pio_id = utarray_len( state->samples );
        utarray_push_back( state->samples, &state->cur_sample );
    }
    state->field = 0;
}

pio_status_t
parse_samples(FILE *fam_fp, UT_array *sample)
{
    char read_buffer[ BUFFER_SIZE ];
    struct state_t state = { 0 };
    struct csv_parser parser;

    state.samples = sample;

    csv_init( &parser, 0 );
    csv_set_delim_func( &parser, fam_is_delim );
    csv_set_delim( &parser, ' ' );
    while( !feof( fam_fp ) )
    {
        int bytes_read = fread( &read_buffer[ 0 ], sizeof( char ), BUFFER_SIZE, fam_fp );
        csv_parse( &parser, read_buffer, bytes_read, &new_field, &new_row, (void *) &state );
    }

    csv_fini( &parser, new_field, new_row, (void *) &state );
    csv_free( &parser );
    
    return ( state.any_error == 0 ) ? PIO_OK : PIO_ERROR;
}
