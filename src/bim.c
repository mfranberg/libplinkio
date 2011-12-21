#include <stdio.h>
#include <utarray.h>

#include <bim.h>
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
 * Properties of the locus array for dtarray.
 */
UT_icd LOCUS_ICD = { sizeof( struct pio_locus_t ), NULL, NULL, NULL };

/**
 * Reads a single line from the input file and stores it in the given
 * buffer.
 *
 * @param fp Pointer to a bim file.
 * @param buffer Buffer to store the read line.
 * @param buffer_length Length of the buffer.
 *
 * @return PIO_OK if the line was read successfully, PIO_END if we are
 *                at the end of the file, PIO_ERROR otherwise.
 */
int
read_locus(FILE *fp, char *buffer, unsigned int buffer_length)
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
 * a locus from the beginning.
 *
 * Note: If data could not be parsed, the contents of
 *       locus is undetermined.
 *
 * @param data The data to be parsed.
 * @param locus The parsed data will be stored here.
 *
 * @return PIO_OK if the locus could be parsed,
 *         PIO_ERROR otherwise.
 */
int
parse_locus(const char *data, struct pio_locus_t *locus)
{
    unsigned int chromosome;
    int num_read_fields = sscanf( data, "%u %s %lu %lu %c %c",
                                &chromosome,
                                locus->name,
                                &locus->position,
                                &locus->bp_position,
                                &locus->major,
                                &locus->minor
                                );
    locus->chromosome = (unsigned char) chromosome;

    if( num_read_fields != 6 )
    {
        return PIO_ERROR;
    }
    else
    {
        return PIO_OK;
    }
}

/**
 * Parses the loci and points the given locus array to a
 * the memory that contains them, and writes back the number
 * of loci.
 *
 * @param bim_file Bim file.
 *
 * @return PIO_OK if the loci could be parsed, PIO_ERROR otherwise.
 */
int
parse_loci(struct pio_bim_file_t *bim_file)
{
    UT_array *loci;
    char read_buffer[BUFFER_SIZE];
    struct pio_locus_t locus;
    
    utarray_new( loci, &LOCUS_ICD );

    while( read_locus( bim_file->fp, read_buffer, BUFFER_SIZE ) != PIO_END )
    {
        if( parse_locus( read_buffer, &locus ) != PIO_OK )
        {
            continue;
        }

        locus.pio_id = utarray_len( loci );
        utarray_push_back( loci, &locus );
    }

    bim_file->num_loci = utarray_len( loci );
    bim_file->locus = (struct pio_locus_t *) utarray_front( loci );
    
    // Free the dtarray but keep the underlying array, if
    // changes are made to the utarray, we need to make sure
    // that no memory is leaked here
    free( loci );

    return PIO_OK;
}

int
bim_open(struct pio_bim_file_t *bim_file, const char *path)
{
    FILE *bim_fp = fopen( path, "r" );
    if( bim_fp == NULL )
    {
        return PIO_ERROR;
    }

    bim_file->fp = bim_fp;
    int status = parse_loci( bim_file );
    fclose( bim_fp );

    return status;
}

struct pio_locus_t *
bim_get_locus(struct pio_bim_file_t *bim_file, unsigned int pio_id)
{
    if( pio_id < bim_file->num_loci )
    {
        return &bim_file->locus[ pio_id ];
    }
    else
    {
        return NULL;
    }
}

void
bim_close(struct pio_bim_file_t *bim_file)
{
    free( bim_file->locus );
    bim_file->locus = NULL;
    bim_file->num_loci = 0;
    bim_file->fp = NULL;
}
