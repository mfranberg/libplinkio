#include <stdio.h>

#include <plinkio/utarray.h>
#include <plinkio/bim.h>
#include <plinkio/bim_parse.h>
#include <plinkio/status.h>

/**
 * Creates mock versions of IO functions to allow unit testing.
 */
#ifdef UNIT_TESTING
    extern FILE *mock_fopen(const char *path, const char *mode);
    int mock_fclose(FILE *fp);

    #define fopen mock_fopen
    #define fclose mock_fclose
#endif

/**
 * Locus destructor. Ensures that the allocated
 * strings are freed properly.
 *
 * @param element Pointer to a locus.
 */
void
utarray_locus_dtor(void *element)
{
    struct pio_locus_t *locus = (struct pio_locus_t *) element;

    if( locus->name != NULL )
    {
        free( locus->name );
    }
    if( locus->allele1 != NULL )
    {
        free( locus->allele1 );
    }
    if( locus->allele2 != NULL )
    {
        free( locus->allele2 );
    }
}

/**
 * Properties of the locus array for dtarray.
 */
UT_icd LOCUS_ICD = { sizeof( struct pio_locus_t ), NULL, NULL, utarray_locus_dtor };

pio_status_t
bim_open(struct pio_bim_file_t *bim_file, const char *path)
{
    int status;
    FILE *bim_fp;
    bzero( bim_file, sizeof( *bim_file ) );
    bim_fp = fopen( path, "r" );
    if( bim_fp == NULL )
    {
        return PIO_ERROR;
    }

    bim_file->fp = bim_fp;
    utarray_new( bim_file->locus, &LOCUS_ICD );
    status = parse_loci( bim_file->fp, bim_file->locus );

    fclose( bim_fp );

    return status;
}

struct pio_locus_t *
bim_get_locus(struct pio_bim_file_t *bim_file, size_t pio_id)
{
    return (struct pio_locus_t *) utarray_eltptr( bim_file->locus, pio_id );  
}

size_t
bim_num_loci(struct pio_bim_file_t *bim_file)
{
    return utarray_len( bim_file->locus );
}

void
bim_close(struct pio_bim_file_t *bim_file)
{
    if( bim_file->locus == NULL )
    {
        return;
    }

    utarray_free( bim_file->locus );
    bim_file->locus = NULL;
    bim_file->fp = NULL;
}
