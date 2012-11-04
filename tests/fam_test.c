#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmockery.h>

#include <fam.h>
#include <fam.c>
#include <fam_parse.c>

/**
 * Returns the smallest value of x and y
 * compared with the < operator.
 */
#define MIN(x,y) ( ( (x) < (y) ) ? (x) : (y) )

/**
 * Number of samples to parse.
 */
#define NUM_SAMPLES 2

/**
 * Sample string to be parsed by libcsv.
 */
const char *TEST_MULTIPLE_SAMPLES = "F1 P1 0 0 1 1\nF1\t P2 0 0 2 2";

/**
 * Start index of next mock_fread call.
 */
int g_sample_pos = 0;

/**
 * Mocks fopen, always returns stdin.
 */
FILE *
mock_fopen(const char *path, const char *mode)
{
    return stdin;
}

/**
 * Mocks fclose, always returns 0.
 */
int
mock_fclose(FILE *fp)
{
    return 0;
}

/**
 * Mocks feof, returns 1 when g_sample_index is >= NUM_SAMPLES. 
 */
int
mock_feof(FILE *stream)
{
    return g_sample_pos >= strlen( TEST_MULTIPLE_SAMPLES );
}

/**
 * Mock fgets, returns a sample as long as it has been called less than
 * NUM_SAMPLES times.
 *
 * Note: You have to reset g_sample_index if you want to use this in
 *       multiple tests.
 */
size_t
mock_fread(void *p, size_t size, size_t nmemb, FILE *stream)
{
    size_t length_left = strlen( TEST_MULTIPLE_SAMPLES ) - g_sample_pos;
    size_t bytes_to_copy = MIN( size * nmemb, length_left );
    g_sample_pos += bytes_to_copy;

    if( bytes_to_copy > 0 )
    {
        strncpy( p, TEST_MULTIPLE_SAMPLES, bytes_to_copy );
        return bytes_to_copy;
    }
    else
    {
        return 0;
    }
}

/**
 * Tests that iids are correctly parsed.
 */
void
test_parse_iid(void **state)
{
    const char *TEST_STRING = "F1";
    pio_status_t status;
    char *iid = parse_iid( TEST_STRING, strlen( TEST_STRING ), &status );

    assert_int_equal( status, PIO_OK );
    assert_string_equal( iid, TEST_STRING );
    free( iid );
}

/**
 * Tests that sex is correctly parsed.
 */
void
test_parse_sex(void **state)
{
    const char *TEST_STRING_MALE = "1";
    const char *TEST_STRING_FEMALE = "2";
    const char *TEST_STRING_UNKOWN = "0";
    pio_status_t status;
    enum sex_t sex;

    sex = parse_sex( TEST_STRING_MALE, strlen( TEST_STRING_MALE ), &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sex, PIO_MALE );

    sex = parse_sex( TEST_STRING_FEMALE, strlen( TEST_STRING_FEMALE ), &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sex, PIO_FEMALE );

    sex = parse_sex( TEST_STRING_UNKOWN, strlen( TEST_STRING_UNKOWN ), &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sex, PIO_UNKOWN );
}

/**
 * Tests that a phenotype is correctly parsed.
 */
void
test_parse_phenotype(void **state)
{
    const char *TEST_STRING_CONTROL = "1";
    const char *TEST_STRING_CASE = "2";
    const char *TEST_STRING_PHENOTYPE = "1.0";
    struct pio_sample_t sample;
    pio_status_t status;

    parse_phenotype( TEST_STRING_CONTROL, strlen( TEST_STRING_CONTROL ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_CONTROL );

    parse_phenotype( TEST_STRING_CASE, strlen( TEST_STRING_CASE ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_CASE );
    
    parse_phenotype( TEST_STRING_PHENOTYPE, strlen( TEST_STRING_PHENOTYPE ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_CONTINUOUS );
    assert_true( fabs( sample.phenotype - 1.0 ) <= 1e-6 );
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
    
    assert_int_equal( fam_open( &fam_file, "" ), PIO_OK );
    assert_int_equal( fam_num_samples( &fam_file ), NUM_SAMPLES );

    person = *fam_get_sample( &fam_file, 0 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P1" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_MALE );
    assert_int_equal( person.affection, PIO_CONTROL );

    person = *fam_get_sample( &fam_file, 1 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P2" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_FEMALE );
    assert_int_equal( person.affection, PIO_CASE );
 
    fam_close( &fam_file );
}

/**
 * Cmockerys initial implementation couldn't handle realloc,
 * this test make sure that it works as intended.
 */
void
test_utarray(void **state)
{
    UT_array *samples;
    struct pio_sample_t person1 = {0};
    struct pio_sample_t person2 = {0};
    utarray_new( samples, &SAMPLE_ICD );

    utarray_push_back( samples, &person1 );
    utarray_push_back( samples, &person2 );

    utarray_free( samples );
}

int main(int argc, char* argv[])
{
    const UnitTest tests[] = {
        unit_test( test_parse_iid ),
        unit_test( test_parse_sex ),
        unit_test( test_parse_phenotype ),
        unit_test( test_parse_multiple_samples ),
        unit_test( test_utarray ),
    };

    return run_tests( tests );
}
