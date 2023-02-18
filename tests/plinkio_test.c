#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <plinkio/plinkio.h>

/**
 * A small test program that reads the given plink file
 * and asserts that the number of rows is correct, depending
 * on how the genotypes are stored.
 */
int
main(int argc, char *argv[])
{
    size_t num_rows;
    snp_t *snp_buffer;
    struct pio_file_t plink_file;

    if( argc != 2 )
    {
        printf("Usage: plinkio_test plink_path_prefix\n");
        return EXIT_FAILURE;
    }

    if( pio_open( &plink_file, argv[ 1 ] ) != PIO_OK )
    {
        printf( "Error: Could not open %s\n", argv[ 1 ] );
        return EXIT_FAILURE;
    }

    num_rows = 0;
    snp_buffer = (snp_t *) malloc( pio_row_size( &plink_file ) );
    while( pio_next_row( &plink_file, snp_buffer ) == PIO_OK )
    {
        num_rows++;
    }

    if( pio_one_locus_per_row( &plink_file ) )
    {
        assert( num_rows == bim_num_loci( &plink_file.bim_file ) );
    }
    else
    {
        assert( num_rows == fam_num_samples( &plink_file.fam_file ) );
    }

    pio_close( &plink_file );
    free( snp_buffer );

    return EXIT_SUCCESS;
}
