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
#include "plink_txt_parse.c"

#include "mock.h"

/**
 * Tests that iids are correctly parsed.
 */
void
test_parse_iid(void **state)
{
    UNUSED_PARAM(state);
    const char *TEST_STRING = "F1";
    pio_status_t status;
    char *iid = libplinkio_parse_str_( TEST_STRING, strlen( TEST_STRING ), &status );

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
    UNUSED_PARAM(state);
    const char *TEST_STRING_MALE = "1";
    const char *TEST_STRING_FEMALE = "2";
    const char *TEST_STRING_UNKNOWN = "0";
    pio_status_t status;
    enum sex_t sex;

    sex = libplinkio_parse_sex_( TEST_STRING_MALE, strlen( TEST_STRING_MALE ), &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sex, PIO_MALE );

    sex = libplinkio_parse_sex_( TEST_STRING_FEMALE, strlen( TEST_STRING_FEMALE ), &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sex, PIO_FEMALE );

    sex = libplinkio_parse_sex_( TEST_STRING_UNKNOWN, strlen( TEST_STRING_UNKNOWN ), &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sex, PIO_UNKNOWN );
}

/**
 * Tests that a phenotype is correctly parsed.
 */
void
test_parse_phenotype(void **state)
{
    UNUSED_PARAM(state);
    const char *TEST_STRING_CONTROL = "1";
    const char *TEST_STRING_CASE = "2";
    const char *TEST_STRING_PHENOTYPE = "1.0";
    const char *TEST_STRING_MISSING = "-9";
    const char *TEST_STRING_MISSING_NA = "NA";
    struct pio_sample_t sample;
    pio_status_t status;

    libplinkio_parse_phenotype_( TEST_STRING_CONTROL, strlen( TEST_STRING_CONTROL ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_CONTROL );

    libplinkio_parse_phenotype_( TEST_STRING_CASE, strlen( TEST_STRING_CASE ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_CASE );
    
    libplinkio_parse_phenotype_( TEST_STRING_PHENOTYPE, strlen( TEST_STRING_PHENOTYPE ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_CONTINUOUS );
    assert_true( fabs( sample.phenotype - 1.0 ) <= 1e-6 );

    libplinkio_parse_phenotype_( TEST_STRING_MISSING, strlen( TEST_STRING_MISSING ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_MISSING );
    assert_true( fabs( sample.phenotype - (-9.0) ) <= 1e-6 );
    
    libplinkio_parse_phenotype_( TEST_STRING_MISSING_NA, strlen( TEST_STRING_MISSING_NA ), &sample, &status );
    assert_int_equal( status, PIO_OK );
    assert_int_equal( sample.affection, PIO_MISSING );
    assert_true( fabs( sample.phenotype - (-9.0) ) <= 1e-6 );
}

/**
 * Tests the parsing of multiple samples. Since parse_samples uses
 * IO functions, we need to have mocked versions for these.
 */
void
test_parse_multiple_samples(void **state)
{
    UNUSED_PARAM(state);
    struct pio_sample_t person;
    struct pio_fam_file_t fam_file;


    mock_init( "F1 P1 0 0 1 1\nF1\t P2 0 0 2 2" );
    assert_int_equal( fam_open( &fam_file, "" ), PIO_OK );
    assert_int_equal( fam_num_samples( &fam_file ), 2 );

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
    UNUSED_PARAM(state);
    UT_array *samples;
    struct pio_sample_t person1 = {0};
    struct pio_sample_t person2 = {0};
    utarray_new( samples, &LIBPLINKIO_SAMPLE_ICD_ );

    utarray_push_back( samples, &person1 );
    utarray_push_back( samples, &person2 );

    utarray_free( samples );
}

int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
    const UnitTest tests[] = {
        unit_test( test_parse_iid ),
        unit_test( test_parse_sex ),
        unit_test( test_parse_phenotype ),
        unit_test( test_parse_multiple_samples ),
        unit_test( test_utarray ),
    };

    return run_tests( tests );
}
