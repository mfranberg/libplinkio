#include "plinkio_private.h"
#include "plink_txt_parse.h"
#include <stdint.h>

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
char*
pio_parse_str(const char *field, size_t length, pio_status_t *status)
{
    if( length > 0 )
    {
        char *iid = (char *) malloc( sizeof( char ) * ( length + 1 ) );
        strncpy( iid, field, length );
        iid[length] = '\0';

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
unsigned char
pio_parse_chr(const char *field, size_t length, pio_status_t *status)
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
float
pio_parse_genetic_position(const char *field, size_t length, pio_status_t *status)
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
long long
pio_parse_bp_position(const char *field, size_t length, pio_status_t *status)
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
 * Parses sex from a csv field.
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param status Status of the conversion.
 *
 * @return The parsed csv field, or PIO_UNKNOWN along with
 *         status = PIO_ERROR if it could not be parsed.
 */
enum sex_t
pio_parse_sex(const char *field, size_t length, pio_status_t *status)
{
    if( length != 1 )
    {
        *status = PIO_ERROR;
        return PIO_UNKNOWN;
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
        return PIO_UNKNOWN;
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
void
pio_parse_phenotype(const char *field, size_t length, struct pio_sample_t *sample, pio_status_t *status)
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
    if( strncmp( field, "-9", length ) == 0 || strncmp( field, "NA", length ) == 0)
    {
        sample->affection = PIO_MISSING;
        sample->phenotype = -9.0f;
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

pio_status_t
pio_txt_parser_init(
    pio_txt_parser_t* parser,
    pio_txt_parser_mode_t parser_mode
)
{
    parser->mode = parser_mode;
    parser->prev_state = PIO_TOKEN_DELIM;
    parser->field_buffer_size = 16;
    parser->field_buffer = (char*)malloc(sizeof(char)*parser->field_buffer_size);
    if (parser->field_buffer == NULL) {
        return PIO_ERROR;
    }
    memset((void*)parser->field_buffer, '\0', sizeof(char)*parser->field_buffer_size);
    parser->field_length = 0;
    parser->field_num = 0;
    parser->row_num = 0;
    return PIO_OK;
}

pio_status_t
pio_txt_parse(
    pio_txt_parser_t* parser,
    char* buffer,
    size_t length,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
)
{
    pio_txt_parser_state_t state = PIO_TOKEN_INIT;
    if (
        parser == NULL
        || parser->field_buffer == NULL
        || buffer == NULL
    )
    {
        return PIO_ERROR;
    }

    for (size_t i = 0; i < length; i++)
    {
        switch (buffer[i]) {
          case ' ':
          case '\t':
              state = PIO_TOKEN_DELIM;
              break;
          case '\n':
              state = PIO_TOKEN_EOL;
              break;
          case '\0':
              return PIO_OK;
              break;
          default:
              state = PIO_TOKEN_GRAPH;
              break;
        }

        if (parser->mode == PIO_SIMPLE_PARSER || parser->field_num < 6) {
            if (state == PIO_TOKEN_GRAPH) {
                size_t least_required_size = parser->field_length + 2;
                if (least_required_size > parser->field_buffer_size) {
                    size_t new_buffer_size = pio_bits_msb_size(least_required_size);
                    if (SIZE_MAX >> 2 < new_buffer_size) return PIO_ERROR;
                    new_buffer_size <<= 2;
                    char* tmp = (char*)realloc((void*)parser->field_buffer, sizeof(char)*new_buffer_size);
                    if (tmp != NULL) {
                        memset((void*)(tmp + parser->field_buffer_size), '\0', sizeof(char)*(new_buffer_size - parser->field_buffer_size));
                        parser->field_buffer = tmp;
                        parser->field_buffer_size = new_buffer_size;
                    } else {
                        return PIO_ERROR;
                    }
                }
                (parser->field_length)++;
                parser->field_buffer[parser->field_length - 1] = buffer[i];
            } else if (parser->prev_state == PIO_TOKEN_GRAPH) {
                new_field(parser->field_buffer, parser->field_length, parser->field_num, data);
                memset((void*)(parser->field_buffer), '\0', sizeof(char)*(parser->field_length));
                parser->field_length = 0;
                parser->field_num++;
            }
        } else {
            if (parser->prev_state == PIO_TOKEN_GRAPH) {
                new_field(parser->field_buffer, parser->field_length, parser->field_num, data);
                memset((void*)(parser->field_buffer), '\0', sizeof(char)*(parser->field_length));
                parser->field_length = 0;
                parser->field_num++;
            }
            if (state == PIO_TOKEN_GRAPH) {
                parser->field_buffer[0] = buffer[i];
                parser->field_length = 1;
            }
        }

        if (state == PIO_TOKEN_EOL) {
            new_row(parser->row_num, data);
            parser->row_num++;
            parser->field_num = 0;
        }

        parser->prev_state = state;
    }
    return PIO_OK;
}

pio_status_t
pio_txt_parse_fini(
    pio_txt_parser_t* parser,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
)
{
    if (parser == NULL || parser->field_buffer == NULL )
    {
        return PIO_ERROR;
    }

    if (parser->prev_state == PIO_TOKEN_GRAPH) {
        new_field(parser->field_buffer, parser->field_length, parser->field_num, data);
    }
    if (parser->prev_state != PIO_TOKEN_EOL) {
        new_row(parser->row_num, data);
    }
    memset((void*)(parser->field_buffer), '\0', sizeof(char)*(parser->field_length));
    parser->field_length = 0;
    parser->field_num = 0;
    parser->row_num = 0;
    parser->prev_state = PIO_TOKEN_EOF;
    return PIO_OK;
}

void
pio_txt_parser_free(
    pio_txt_parser_t* parser
)
{
    parser->field_buffer_size = 0;
    free(parser->field_buffer);
    parser->field_buffer = NULL;
}
