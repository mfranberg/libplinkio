#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <plinkio.h>

/**
 * A small test program that reads the given plink file
 * and asserts that the number of rows is correct, depending
 * on how the genotypes are stored.
 */
int
main(int argc, char *argv[])
{
    int num_rows;
    unsigned char *snp_buffer;
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
    snp_buffer = pio_allocate_row_buffer( &plink_file );
    while( pio_next_row( &plink_file, snp_buffer ) == PIO_OK )
    {
        num_rows++;
    }

    if( pio_row_order( &plink_file ) == ONE_LOCUS_PER_ROW )
    {
        assert( num_rows == plink_file.bim_file.num_loci );
    }
    else
    {
        assert( num_rows == plink_file.fam_file.num_samples );
    }

    pio_close( &plink_file );
    free( snp_buffer );

    return EXIT_SUCCESS;
}
