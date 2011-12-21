#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmockery.h>

#include <bim.h>
#include <bim.c>

/**
 * Number of loci to parse.
 */
#define NUM_LOCI 2

/**
 * Index of locus to be returned by mock_fgets.
 */
int g_locus_index = 0;

/**
 * Mocks feof, returns 1 when g_locus_index is >= NUM_LOCI. 
 */
int
mock_feof(FILE *stream)
{
    return g_locus_index <= NUM_LOCI;
}

/**
 * Mock fgets, returns a locus as long as it has been called less than
 * NUM_LOCI times.
 *
 * Note: You have to reset g_locus_index if you want to use this in
 *       multiple tests.
 */
char *
mock_fgets(char *s, int n, FILE *stream)
{
    const char *locus_list[NUM_LOCI] = { "1 rs1 0 1234567 A C", "1 rs2 0 7654321 G T" };

    if( g_locus_index < NUM_LOCI )
    {
        strncpy( s, locus_list[ g_locus_index ], n );
        return (char *) locus_list[ g_locus_index++ ];
    }
    else
    {
        return NULL;
    }

}

/**
 * Tests the parsing of a correct locus.
 */
void
test_parse_locus(void **state)
{
    const char *TEST_STRING = "1 rs1 0 1234567 A C";
    
    struct pio_locus_t locus;
    assert_int_equal( parse_locus( TEST_STRING, &locus ), PIO_OK );
    assert_int_equal( locus.chromosome, 1 ); 
    assert_string_equal( locus.name, "rs1" );
    assert_int_equal( locus.position, 0 );
    assert_int_equal( locus.bp_position, 1234567 );
    assert_int_equal( locus.major, 'A' );
    assert_int_equal( locus.minor, 'C' );
}

/**
 * Tests that parsing fails when a field is missing.
 */
void test_parse_locus_fail(void **state)
{
    const char *TEST_STRING = "1 rs1 0 0 A";
    
    struct pio_locus_t locus;
    assert_int_equal( parse_locus( TEST_STRING, &locus ), PIO_ERROR );
}

/**
 * Tests the parsing of multiple loci. Since parse_loci uses
 * IO functions, we need to have mocked versions for these.
 */
void
test_parse_multiple_loci(void **state)
{
    struct pio_bim_file_t bim_file;
    assert_int_equal( parse_loci( &bim_file ), PIO_OK );
    assert_int_equal( bim_file.num_loci, NUM_LOCI );

    struct pio_locus_t locus = bim_file.locus[0];
    assert_int_equal( locus.chromosome, 1 ); 
    assert_string_equal( locus.name, "rs1" );
    assert_int_equal( locus.position, 0 );
    assert_int_equal( locus.bp_position, 1234567 );
    assert_int_equal( locus.major, 'A' );
    assert_int_equal( locus.minor, 'C' );

    locus = bim_file.locus[1];
    assert_int_equal( locus.chromosome, 1 ); 
    assert_string_equal( locus.name, "rs2" );
    assert_int_equal( locus.position, 0 );
    assert_int_equal( locus.bp_position, 7654321 );
    assert_int_equal( locus.major, 'G' );
    assert_int_equal( locus.minor, 'T' );
}

int main(int argc, char* argv[])
{
    const UnitTest tests[] = {
        unit_test( test_parse_locus ),
        unit_test( test_parse_locus_fail ),
        unit_test( test_parse_multiple_loci ),
    };

    return run_tests( tests );
}
