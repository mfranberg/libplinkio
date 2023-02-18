#include "private/utility.h"
#include "private/plink_txt_parse.h"
#include <stdint.h>

/**
 * Buffer size for reading CSV file.
 */
#define LIBPLINKIO_COLUMN_COUNT_BUFFER_SIZE_ 4096

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
libplinkio_parse_str_(const char *field, size_t length, pio_status_t *status)
{
    if( length > 0 )
    {
        char *iid = (char *) calloc(length + 1, sizeof(char));
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
libplinkio_parse_chr_(const char *field, size_t length, pio_status_t *status)
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
libplinkio_parse_genetic_position_(const char *field, size_t length, pio_status_t *status)
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
libplinkio_parse_bp_position_(const char *field, size_t length, pio_status_t *status)
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
libplinkio_parse_sex_(const char *field, size_t length, pio_status_t *status)
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
libplinkio_parse_phenotype_(const char *field, size_t length, struct pio_sample_t *sample, pio_status_t *status)
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

/**
 * Parses a phenotype from a csv field.
 *
 * @param field Csv field.
 * @param length Length of the field.
 * @param idx Index of the locus field.
 * @param loci Loci.
 * @param prev_call Previous call type of the allele.
 * @param status Status of the conversion.
 */
void
libplinkio_parse_allele_(const char *field, size_t length, size_t locus_idx, size_t allele_idx, libplinkio_loci_private_t loci, libplinkio_allele_call_private_t* prev_call, snp_t* snps, pio_status_t* status)
{
    libplinkio_allele_call_private_t call = LIBPLINKIO_ALLELE_CALL_NO_;

    struct pio_locus_t* locus = libplinkio_get_locus_(loci, locus_idx);
    char* allele = (char*)calloc(length + 1, sizeof(char));
    if (allele == NULL) goto error;
    strncpy( allele, field, length );
    allele[length] = '\0';

    if (strcmp(allele, "0") == 0) {
        free(allele);
        call = LIBPLINKIO_ALLELE_CALL_NO_;
    } else if (locus->allele1 == NULL) {
        locus->allele1 = allele;
        call = LIBPLINKIO_ALLELE_CALL_1_;
    } else if (strcmp(allele, locus->allele1) == 0) {
        free(allele);
        call = LIBPLINKIO_ALLELE_CALL_1_;
    } else if (locus->allele2 == NULL) {
        locus->allele2 = allele;
        call = LIBPLINKIO_ALLELE_CALL_2_;
    } else if (strcmp(allele, locus->allele2) == 0) {
        free(allele);
        call = LIBPLINKIO_ALLELE_CALL_2_;
    } else {
        goto error;
    }
    allele = NULL;

    if (allele_idx == 0) {
        // first call
        *prev_call = call;
    } else {
        if (*prev_call == LIBPLINKIO_ALLELE_CALL_ERROR_) {
            // error
            goto error;
        } else if (call == LIBPLINKIO_ALLELE_CALL_NO_ || *prev_call == LIBPLINKIO_ALLELE_CALL_NO_) {
            // no call
            snps[locus_idx] = 3;
        } else if (*prev_call == call) {
            // homozygous
            if (call == LIBPLINKIO_ALLELE_CALL_1_) {
                // allele 1
                snps[locus_idx] = 0;
            } else {
                // allele 2
                snps[locus_idx] = 2;
            }
        } else {
            // heterozygous
            snps[locus_idx] = 1;
        }
    }
    *status = PIO_OK;
    return;
error:
    free(allele);
    *prev_call = LIBPLINKIO_ALLELE_CALL_ERROR_;
    *status = PIO_ERROR;
    return;
}

pio_status_t
libplinkio_txt_parser_init_(
    libplinkio_txt_parser_private_t* parser
)
{
    parser->prev_state = LIBPLINKIO_CHAR_SET_DELIM_;
    parser->field_buffer_size = 16;
    parser->field_buffer = (char*)calloc(parser->field_buffer_size, sizeof(char));
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
libplinkio_txt_parse_(
    libplinkio_txt_parser_private_t* parser,
    char* buffer,
    size_t length,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
)
{
    libplinkio_txt_parser_state_private_t state = LIBPLINKIO_CHAR_SET_INIT_;
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
              state = LIBPLINKIO_CHAR_SET_DELIM_;
              break;
          case '\n':
              state = LIBPLINKIO_CHAR_SET_EOL_;
              break;
          case '\0':
              return PIO_OK;
              break;
          default:
              state = LIBPLINKIO_CHAR_SET_GRAPH_;
              break;
        }

        if (state == LIBPLINKIO_CHAR_SET_GRAPH_) {
            size_t least_required_size = parser->field_length + 2;
            if (least_required_size > parser->field_buffer_size) {
                size_t new_buffer_size = libplinkio_bits_msb_size_(least_required_size);
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
        } else if (parser->prev_state == LIBPLINKIO_CHAR_SET_GRAPH_) {
            new_field(parser->field_buffer, parser->field_length, parser->field_num, data);
            memset((void*)(parser->field_buffer), '\0', sizeof(char)*(parser->field_length));
            parser->field_length = 0;
            parser->field_num++;
        }

        if (state == LIBPLINKIO_CHAR_SET_EOL_) {
            new_row(parser->row_num, data);
            parser->row_num++;
            parser->field_num = 0;
        }

        parser->prev_state = state;
    }
    return PIO_OK;
}

pio_status_t
libplinkio_txt_parse_fini_(
    libplinkio_txt_parser_private_t* parser,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
)
{
    if (parser == NULL || parser->field_buffer == NULL )
    {
        return PIO_ERROR;
    }

    if (parser->prev_state == LIBPLINKIO_CHAR_SET_GRAPH_) {
        new_field(parser->field_buffer, parser->field_length, parser->field_num, data);
    }
    if (parser->prev_state != LIBPLINKIO_CHAR_SET_EOL_) {
        new_row(parser->row_num, data);
    }
    memset((void*)(parser->field_buffer), '\0', sizeof(char)*(parser->field_length));
    parser->field_length = 0;
    parser->field_num = 0;
    parser->row_num = 0;
    parser->prev_state = LIBPLINKIO_CHAR_SET_EOF_;
    return PIO_OK;
}

void
libplinkio_txt_parser_free_(
    libplinkio_txt_parser_private_t* parser
)
{
    parser->field_buffer_size = 0;
    free(parser->field_buffer);
    parser->field_buffer = NULL;
}

int libplinkio_count_txt_column_(FILE* fp) {
    char read_buffer[ LIBPLINKIO_COLUMN_COUNT_BUFFER_SIZE_ ];
    libplinkio_txt_parser_state_private_t prev_state = LIBPLINKIO_CHAR_SET_INIT_;
    int column = 0;

    while( !feof( fp ) ) {
        size_t bytes_read = fread( read_buffer, sizeof( char ), LIBPLINKIO_COLUMN_COUNT_BUFFER_SIZE_ - 1, fp );
        if (ferror(fp)) goto error;
        read_buffer[bytes_read] = '\0';
        for (size_t i = 0; i < bytes_read; i++) {
            switch (read_buffer[i]) {
                case ' ':
                case '\t':
                    prev_state = LIBPLINKIO_CHAR_SET_DELIM_;
                    break;
                case '\n':
                case '\0':
                    goto eol;
                    break;
                default:
                    if (prev_state != LIBPLINKIO_CHAR_SET_GRAPH_) column += 1;
                    prev_state = LIBPLINKIO_CHAR_SET_GRAPH_;
                    break;
            }
        }
    } eol:
    rewind(fp);
    return column;
error:
    return -1;
}