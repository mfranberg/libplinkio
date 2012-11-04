#include <plinkio/plinkio.h>

/**
 * Simple test program for transpose.
 */
int
main(int argc, char *argv[])
{
    if( argc != 3 )
    {
        printf( "Usage: transpose plink_prefix plink_transposed_prefix\n" );
        exit( 1 );
    }

    pio_transpose( argv[ 1 ], argv[ 2 ] );

    return 0;
}
