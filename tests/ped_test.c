#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cmockery.h>

#undef UNIT_TESTING

#include <plinkio/plinkio.h>
#include <bed.h>
#include <bim.h>
#include <fam.h>
#include "private/bed.h"
#include "private/map.h"
#include "private/ped.h"
#include "private/locus.h"
#include "private/sample.h"

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
 * Tests the parsing of multiple samples.
 */
void
test_parse_multiple_samples(void **state)
{
    UNUSED_PARAM(state);
    libplinkio_loci_private_t loci = libplinkio_init_loci_();
    libplinkio_samples_private_t samples = libplinkio_init_samples_();
    struct pio_locus_t locus = {0};
    struct pio_sample_t person = {0};
    struct pio_file_t plink_file = {0};
    size_t num_loci = 0;
    size_t num_samples = 0;

    assert_int_equal( libplinkio_map_open_( &loci, "./data/small.map" ), PIO_OK );
    assert_int_equal( num_loci = libplinkio_get_num_loci_( loci ), 2 );

    assert_int_equal(
        libplinkio_bed_tmp_transposed_create_(&plink_file.bed_file, "./data/small.bed", num_loci),
        PIO_OK
    );
    assert_int_equal( libplinkio_ped_open_( &samples, &loci, &plink_file.bed_file, "./data/small.ped" ), PIO_OK );
    assert_int_equal( num_samples = libplinkio_get_num_samples_( samples ), 4 );

    person = *libplinkio_get_sample_( samples, 0 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P1" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_MALE );
    assert_int_equal( person.affection, PIO_CONTROL );

    person = *libplinkio_get_sample_( samples, 1 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P2" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_FEMALE );
    assert_int_equal( person.affection, PIO_CASE );

    locus = *libplinkio_get_locus_( loci, 0 );
    assert_string_equal( locus.allele1, "A" );
    assert_string_equal( locus.allele2, "T" );

    locus = *libplinkio_get_locus_( loci, 1 );
    assert_string_equal( locus.allele1, "G" );
    assert_string_equal( locus.allele2, "C" );

    assert_int_equal( libplinkio_bed_transpose_pio_bed_file_(&plink_file.bed_file, "./data/small.bed", num_loci, num_samples, false), PIO_OK );
    
    assert_int_equal(libplinkio_flip_alleles_(loci, &plink_file.bed_file, num_samples), PIO_OK);

    assert_int_equal(libplinkio_bim_link_loci_to_file_(loci, &plink_file.bim_file, "./data/small.bim", false), PIO_OK);

    assert_int_equal(libplinkio_fam_link_samples_to_file_(samples, &plink_file.fam_file, "./data/small.fam", false), PIO_OK);
    
    pio_close(&plink_file);
}

/**
 * Tests the parsing of multiple samples.
 */
void
test_parse_multiple_samples_compound(void **state)
{
    UNUSED_PARAM(state);
    libplinkio_loci_private_t loci = libplinkio_init_loci_();
    libplinkio_samples_private_t samples = libplinkio_init_samples_();
    struct pio_locus_t locus = {0};
    struct pio_sample_t person = {0};
    struct pio_file_t plink_file = {0};
    size_t num_loci = 0;
    size_t num_samples = 0;

    assert_int_equal( libplinkio_map_open_( &loci, "./data/small_compound.map" ), PIO_OK );
    assert_int_equal( num_loci = libplinkio_get_num_loci_( loci ), 2 );

    assert_int_equal(
        libplinkio_bed_tmp_transposed_create_(&plink_file.bed_file, "./data/small_compound.bed", num_loci),
        PIO_OK
    );
    assert_int_equal( libplinkio_ped_open_( &samples, &loci, &plink_file.bed_file, "./data/small_compound.ped" ), PIO_OK );
    assert_int_equal( num_samples = libplinkio_get_num_samples_( samples ), 4 );

    person = *libplinkio_get_sample_( samples, 0 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P1" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_MALE );
    assert_int_equal( person.affection, PIO_CONTROL );

    person = *libplinkio_get_sample_( samples, 1 );
    assert_string_equal( person.fid, "F1" );
    assert_string_equal( person.iid, "P2" );
    assert_string_equal( person.father_iid, "0" );
    assert_string_equal( person.mother_iid, "0" );
    assert_int_equal( person.sex, PIO_FEMALE );
    assert_int_equal( person.affection, PIO_CASE );

    locus = *libplinkio_get_locus_( loci, 0 );
    assert_string_equal( locus.allele1, "A" );
    assert_string_equal( locus.allele2, "T" );

    locus = *libplinkio_get_locus_( loci, 1 );
    assert_string_equal( locus.allele1, "G" );
    assert_string_equal( locus.allele2, "C" );

    assert_int_equal( libplinkio_bed_transpose_pio_bed_file_(&plink_file.bed_file, "./data/small_compound.bed", num_loci, num_samples, false), PIO_OK );
    
    assert_int_equal(libplinkio_flip_alleles_(loci, &plink_file.bed_file, num_samples), PIO_OK);

    assert_int_equal(libplinkio_bim_link_loci_to_file_(loci, &plink_file.bim_file, "./data/small_compound.bim", false), PIO_OK);

    assert_int_equal(libplinkio_fam_link_samples_to_file_(samples, &plink_file.fam_file, "./data/small_compound.fam", false), PIO_OK);
    
    pio_close(&plink_file);
}


int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
    const UnitTest tests[] = {
        unit_test( test_parse_multiple_samples ),
        unit_test( test_parse_multiple_samples_compound ),
    };

    return run_tests( tests );
}
