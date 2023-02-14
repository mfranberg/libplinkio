#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmockery.h>

#include "private/map.h"
#include "private/locus.h"
#include "map.c"
#include "map_parse.c"
#include "plink_txt_parse.c"
#include "mock.h"

/**
 * Tests the parsing of multiple loci.
 */
void
test_parse_multiple_loci(void **state)
{
    UNUSED_PARAM(state);
    libplinkio_loci_private_t loci = libplinkio_init_loci_();
    struct pio_locus_t locus;

    mock_init( "1 rs1 0 1234567\n1 rs2 0.23 7654321" );
    assert_int_equal( libplinkio_map_open_( &loci, "" ), PIO_OK );
    assert_int_equal( libplinkio_get_num_loci_( loci ), 2 );

    locus = *libplinkio_get_locus_( loci, 0 );
    assert_int_equal( locus.chromosome, 1 ); 
    assert_string_equal( locus.name, "rs1" );
    assert_true( fabs( locus.position - 0.0 ) <= 1e-6 );
    assert_int_equal( locus.bp_position, 1234567 );

    locus = *libplinkio_get_locus_( loci, 1 );
    assert_int_equal( locus.chromosome, 1 ); 
    assert_string_equal( locus.name, "rs2" );
    assert_true( fabs( locus.position - 0.23 ) <= 1e-6 );
    assert_int_equal( locus.bp_position, 7654321 );

    libplinkio_free_loci_(loci);
}

int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
    const UnitTest tests[] = {
        unit_test( test_parse_multiple_loci ),
    };

    return run_tests( tests );
}
