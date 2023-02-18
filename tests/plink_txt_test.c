#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmockery.h>

#undef UNIT_TESTING

#define LIBPLINKIO_EXPERIMENTAL
#include <plinkio/plinkio.h>

#include "plinkio.c"
#include "file.c"
#include "bed.c"
#include "bed_header.c"
#include "bim.c"
#include "bim_parse.c"
#include "fam.c"
#include "fam_parse.c"
#include "map.c"
#include "map_parse.c"
#include "ped.c"
#include "ped_parse.c"
#include "plink_txt_parse.c"
#include "utility.c"
#include "packed_snp.c"

#define UNIT_TESTING

/**
 * Tests the parsing of text plink files.
 */
void
test_parse_plink_txt(void **state)
{
    UNUSED_PARAM(state);
    struct pio_locus_t locus = {0};
    struct pio_sample_t person = {0};
    struct pio_file_t plink_file = {0};
    size_t num_loci = 0;
    size_t num_samples = 0;

    assert_int_equal( libplinkio_open_txt(&plink_file, "./data/small"), PIO_OK );
    assert_int_equal( num_loci = pio_num_loci(&plink_file), 2 );
    assert_int_equal( num_samples = pio_num_samples(&plink_file), 4 );

    person = *pio_get_sample( &plink_file, 0 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P1" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_MALE );
    assert_int_equal( person.affection, PIO_CONTROL );

    person = *pio_get_sample( &plink_file, 1 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P2" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_FEMALE );
    assert_int_equal( person.affection, PIO_CASE );

    locus = *pio_get_locus( &plink_file, 0 );
    assert_string_equal( locus.allele1, "A" );
    assert_string_equal( locus.allele2, "T" );

    locus = *pio_get_locus( &plink_file, 1 );
    assert_string_equal( locus.allele1, "C" );
    assert_string_equal( locus.allele2, "G" );

    pio_close(&plink_file);
}

int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
    const UnitTest tests[] = {
        unit_test( test_parse_plink_txt ),
    };

    return run_tests( tests );
}
