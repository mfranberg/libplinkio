#include <errno.h>
#include <stdio.h>
#include <utarray.h>

#include <fam.h>
#include <status.h>

/**
 * Creates mock versions of IO functions to allow unit testing.
 */
#ifdef UNIT_TESTING
    extern int mock_feof(FILE *stream);
    extern char *mock_fgets(char *s, int n, FILE *stream);

    #define fgets mock_fgets
    #define feof mock_feof
#endif

/**
 * Size of string buffer for parsing phenotype.
 */
#define BUFFER_SIZE 1024

/**
 * Properties of the sample array for dtarray.
 */
UT_icd SAMPLE_ICD = { sizeof( struct pio_sample_t ), NULL, NULL, NULL };

/**
 * Reads a single line from the input file and stores it in the given
 * buffer.
 *
 * @param fp Pointer to a fam file.
 * @param buffer Buffer to store the read line.
 * @param buffer_length Length of the buffer.
 *
 * @return PIO_OK if the line was read successfully, PIO_END if we are
 *                at the end of the file, PIO_ERROR otherwise.
 */
int
read_sample(FILE *fp, char *buffer, unsigned int buffer_length)
{
    char *result = fgets( buffer, buffer_length, fp );
    if( result != NULL )
    {
        return PIO_OK;
    }
    else
    {
        if( feof( fp ) != 0 )
        {
            return PIO_END;
        }
        else
        {
            return PIO_ERROR;
        }
    }
}

/**
 * Takes the given pointer to data and tries to parse
 * a sample from the beginning.
 *
 * Note: If data could not be parsed, the contents of
 *       locus is undetermined.
 *
 * @param data The data to be parsed.
 * @param locus The parsed data will be stored here.
 *
 * @return PIO_OK if the sample could be parsed,
 *         PIO_ERROR otherwise.
 */
int
parse_sample(const char *sample, struct pio_sample_t *person)
{
    unsigned int sex;
    char phenotype_as_string[BUFFER_SIZE];
    char *endptr;
    long phenotype_int;
    double phenotype_float;

    int num_read_fields = sscanf( sample, "%u %s %u %u %u %s",
                                &person->fid,
                                person->iid,
                                &person->father_iid,
                                &person->mother_iid,
                                &sex,
                                phenotype_as_string );

    if( num_read_fields != 6 )
    {
        return PIO_ERROR;
    }
        
    if( sex == 1 )
    {
        person->sex = MALE;
    }
    else
    {
        person->sex = FEMALE;
    }

    errno = 0;
    phenotype_int = strtol( phenotype_as_string, &endptr, 10 );
    if( errno == 0 && ( endptr == NULL || *endptr == '\0' ) )
    {
        if( phenotype_int == 1L || phenotype_int == 2L )
        {
            person->phenotype.as_int = (int) phenotype_int - 1;
        }
        else
        {
            person->phenotype.as_int = -9;
        }

        person->outcome_type = DISCRETE;
        return PIO_OK;
    }

    errno = 0;
    phenotype_float = strtod( phenotype_as_string, &endptr );
    if( errno == 0 && ( endptr == NULL || *endptr == '\0' ) )
    {
        person->phenotype.as_float = (float) phenotype_float;
        person->outcome_type = CONTINUOUS;
        return PIO_OK;
    }

    return PIO_ERROR;

}

/**
 * Parses the samples and points the given sample array to a
 * the memory that contains them, and writes back the number
 * of samples.
 *
 * @param fam_file Fam file.
 *
 * @return PIO_OK if the samples could be parsed, PIO_ERROR otherwise.
 */
int
parse_samples(struct pio_fam_file_t *fam_file)
{
    UT_array *samples;
    char read_buffer[BUFFER_SIZE];
    struct pio_sample_t person;
    
    utarray_new( samples, &SAMPLE_ICD );

    while( read_sample( fam_file->fp, read_buffer, BUFFER_SIZE ) != PIO_END )
    {
        if( parse_sample( read_buffer, &person ) != PIO_OK )
        {
            continue;
        }

        person.pio_id = utarray_len( samples );
        utarray_push_back( samples, &person );
    }

    fam_file->num_samples = utarray_len( samples );
    fam_file->sample = (struct pio_sample_t *) utarray_front( samples );
    
    /* Free the dtarray but keep the underlying array, if
       changes are made to the utarray, we need to make sure
       that no memory is leaked here. */
    free( samples );

    return PIO_OK;
}

int
fam_open(struct pio_fam_file_t *fam_file, const char *path)
{
    int status;
    FILE *fam_fp = fopen( path, "r" );
    if( fam_fp == NULL )
    {
        return PIO_ERROR;
    }

    fam_file->fp = fam_fp;
    status = parse_samples( fam_file );
    fclose( fam_fp );

    return status;
}

struct pio_sample_t *
fam_get_sample(struct pio_fam_file_t *fam_file, unsigned int pio_id)
{
    if( pio_id < fam_file->num_samples )
    {
        return &fam_file->sample[ pio_id ];
    }
    else
    {
        return NULL;
    }
}

unsigned int
fam_num_samples(struct pio_fam_file_t *fam_file)
{
    return fam_file->num_samples;
}

void
fam_close(struct pio_fam_file_t *fam_file)
{
    free( fam_file->sample );
    fam_file->sample = NULL;
    fam_file->num_samples = 0;
    fam_file->fp = NULL;
}
