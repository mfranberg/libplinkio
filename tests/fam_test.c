#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmockery.h>

#include <fam.h>
#include <fam.c>

/**
 * Number of samples to parse.
 */
#define NUM_SAMPLES 2

/**
 * Index of sample to be returned by mock_fgets.
 */
int g_sample_index = 0;

/**
 * Mocks feof, returns 1 when g_sample_index is >= NUM_SAMPLES. 
 */
int
mock_feof(FILE *stream)
{
    return g_sample_index <= NUM_SAMPLES;
}

/**
 * Mock fgets, returns a sample as long as it has been called less than
 * NUM_SAMPLES times.
 *
 * Note: You have to reset g_sample_index if you want to use this in
 *       multiple tests.
 */
char *
mock_fgets(char *s, int n, FILE *stream)
{
    const char *sample_list[NUM_SAMPLES] = { "0 P1 0 0 0 0", "0 P2 0 0 0 0" };

    if( g_sample_index < NUM_SAMPLES )
    {
        strncpy( s, sample_list[ g_sample_index ], n );
        return (char *) sample_list[ g_sample_index++ ];
    }
    else
    {
        return NULL;
    }

}

/**
 * Tests the parsing of a correct sample.
 */
void
test_parse_sample(void **state)
{
    const char *TEST_STRING = "1 P1 0 0 1 0";
    
    struct pio_sample_t person;
    assert_int_equal( parse_sample( TEST_STRING, &person ), PIO_OK );
    assert_int_equal( person.fid, 1 ); 
    assert_string_equal( person.iid, "P1" );
    assert_int_equal( person.father_iid, 0 );
    assert_int_equal( person.mother_iid, 0 );
    assert_int_equal( person.sex, MALE );
    assert_int_equal( person.phenotype.as_int, 0 );
}

/**
 * Tests the parsing of a correct sample with a
 * real phenotype.
 */
void test_parse_sample_double(void **state)
{
    const char *TEST_STRING = "1 P1 0 0 1 4.5";
    
    struct pio_sample_t person;
    assert_int_equal( parse_sample( TEST_STRING, &person ), PIO_OK );
    assert_int_equal( person.fid, 1 ); 
    assert_string_equal( person.iid, "P1" );
    assert_int_equal( person.father_iid, 0 );
    assert_int_equal( person.mother_iid, 0 );
    assert_int_equal( person.sex, MALE );
    assert_true( fabs( person.phenotype.as_float - 4.5 ) <= 1e-6 );
}

/**
 * Tests that parsing fails when a field is missing.
 */
void test_parse_sample_fail(void **state)
{
    const char *TEST_STRING = "1 P1 0 0 1";
    
    struct pio_sample_t person;
    assert_int_equal( parse_sample( TEST_STRING, &person ), PIO_ERROR );
}

/**
 * Tests the parsing of multiple samples. Since parse_samples uses
 * IO functions, we need to have mocked versions for these.
 */
void
test_parse_multiple_samples(void **state)
{
    struct pio_sample_t person;
    struct pio_fam_file_t fam_file;
    assert_int_equal( parse_samples( &fam_file ), PIO_OK );
    assert_int_equal( fam_file.num_samples, NUM_SAMPLES );

    person = fam_file.sample[0];
    assert_int_equal( person.fid, 0 ); 
    assert_string_equal( person.iid, "P1" );
    assert_int_equal( person.father_iid, 0 );
    assert_int_equal( person.mother_iid, 0 );
    assert_int_equal( person.sex, FEMALE );
    assert_int_equal( person.phenotype.as_int, 0 );

    person = fam_file.sample[1];
    assert_int_equal( person.fid, 0 ); 
    assert_string_equal( person.iid, "P2" );
    assert_int_equal( person.father_iid, 0 );
    assert_int_equal( person.mother_iid, 0 );
    assert_int_equal( person.sex, FEMALE );
    assert_int_equal( person.phenotype.as_int, 0 );
}

int main(int argc, char* argv[])
{
    const UnitTest tests[] = {
        unit_test( test_parse_sample ),
        unit_test( test_parse_sample_double ),
        unit_test( test_parse_sample_fail ),
        unit_test( test_parse_multiple_samples ),
    };

    return run_tests( tests );
}
