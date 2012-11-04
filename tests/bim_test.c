#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmockery.h>

#include <bim.h>
#include <bim.c>
#include <bim_parse.c>
#include "mock.h"

/**
 * Tests the parsing of a position.
 */
void
test_parse_position(void **state)
{
    const char *TEST_STRING1 = "123456";
    const char *TEST_STRING2 = "-1";
    pio_status_t status;
    
    assert_int_equal( parse_bp_position( TEST_STRING1, strlen( TEST_STRING1 ), &status ), 123456LL );
    assert_int_equal( status, PIO_OK );
    
    assert_int_equal( parse_bp_position( TEST_STRING2, strlen( TEST_STRING2 ), &status ), -1LL );
    assert_int_equal( status, PIO_OK );
}

/**
 * Tests the parsing of a position.
 */
void
test_parse_chr(void **state)
{
    const char *TEST_STRING = "16";
    pio_status_t status;
    
    assert_int_equal( parse_chr( TEST_STRING, strlen( TEST_STRING ), &status ), 16 );
    assert_int_equal( status, PIO_OK );
}

/**
 * Tests the parsing of multiple loci. Since parse_loci uses
 * IO functions, we need to have mocked versions for these.
 */
void
test_parse_multiple_loci(void **state)
{
    struct pio_locus_t locus;
    struct pio_bim_file_t bim_file;

    mock_init( "1 rs1 0 1234567 A C\n1 rs2 0.23 7654321 - ACCG" );
    assert_int_equal( bim_open( &bim_file, "" ), PIO_OK );
    assert_int_equal( bim_num_loci( &bim_file ), 2 );

    locus = *bim_get_locus( &bim_file, 0 );
    assert_int_equal( locus.chromosome, 1 ); 
    assert_string_equal( locus.name, "rs1" );
    assert_true( fabs( locus.position - 0.0 ) <= 1e-6 );
    assert_int_equal( locus.bp_position, 1234567 );
    assert_string_equal( locus.allele1, "A" );
    assert_string_equal( locus.allele2, "C" );

    locus = *bim_get_locus( &bim_file, 1 );
    assert_int_equal( locus.chromosome, 1 ); 
    assert_string_equal( locus.name, "rs2" );
    assert_true( fabs( locus.position - 0.23 ) <= 1e-6 );
    assert_int_equal( locus.bp_position, 7654321 );
    assert_string_equal( locus.allele1, "-" );
    assert_string_equal( locus.allele2, "ACCG" );

    bim_close( &bim_file );
}

int main(int argc, char* argv[])
{
    const UnitTest tests[] = {
        unit_test( test_parse_position ),
        unit_test( test_parse_chr ),
        unit_test( test_parse_multiple_loci ),
    };

    return run_tests( tests );
}
