#include <stdio.h>
#include <utarray.h>

#include <fam.h>

/**
 * Size of string buffer for parsing phenotype.
 */
#define BUFFER_SIZE 1024

/**
 * Reads a line from the given file pointer and parses
 * it as a sample, the parsed data is stored in the given
 * sample pointer.
 *
 * @param fp Pointer to a fam file.
 * @param person The parsed data will be stored here.
 *
 * @return 1 if the sample could be parsed, 0 otherwise.
 */
int parse_sample(FILE *fp, struct pio_sample_t *person)
{
    unsigned int sex;
    char phenotype_as_string[BUFFER_SIZE];

    int num_read_fields = fscanf( fp, "%u %u %u %u %u %s",
                                &person->fid,
                                &person->iid,
                                &person->father_iid,
                                &person->mother_iid,
                                &sex,
                                phenotype_as_string );

    if( num_read_fields != 6 )
    {
        return 0;
    }
        
    if( sex == 1 )
    {
        person->sex = MALE;
    }
    else
    {
        person->sex = FEMALE;
    }

    char *endptr;
    long phenotype_int = strtol( phenotype_as_string, &endptr, 10 );
    if( endptr == NULL )
    {
        person->phenotype.as_int = (int) phenotype_int; 
    }
    else
    {
        float phenotype_float = strtof( phenotype_as_string, NULL );
        person->phenotype.as_float = phenotype_float;
    }

    return 1;

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
int parse_samples(struct pio_fam_file_t *fam_file)
{
    UT_icd sample_icd = { sizeof( struct pio_sample_t ), NULL, NULL, NULL }; 
    UT_array *samples;
    unsigned int pio_id = 0;
    struct pio_sample_t person;
    
    utarray_new( samples, &sample_icd );

    while( parse_sample( fam_file->fp, &person ) == 1 )
    {
        person.pio_id = pio_id;
        utarray_push_back( samples, &person );

        pio_id++;
    }

    fam_file->num_samples = utarray_len( samples );
    fam_file->sample = (struct pio_sample_t *) utarray_front( samples );
    
    // Free the dtarray but keep the underlying array, if
    // changes are made to the utarray, we need to make sure
    // that no memory is leaked here
    free( samples );

    return PIO_OK;
}

int
fam_open(struct pio_fam_file_t *fam_file, const char *path)
{
    FILE *fam_fp = fopen( path, "r" );
    if( fam_fp == NULL )
    {
        return PIO_ERROR;
    }

    int status = parse_samples( fam_file );
    fclose( fam_fp );

    return status;
}

void
fam_close(struct pio_fam_file_t *fam_file)
{
    free( fam_file->sample );
    fam_file->sample = NULL;
    fam_file->num_samples = 0;
    fam_file->fp = NULL;
}
