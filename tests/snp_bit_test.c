#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <stdint.h>

#include <cmockery.h>

#include <private/packed_snp.h>
#include <private/utility.h>

#include <packed_snp.c>
#include <utility.c>
#include <bed_header.c>

void
test_cnt_alleles(void **state)
{
    UNUSED_PARAM(state);
    
    uint8_t test_data[] = {
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001
    };
    size_t num_cols = sizeof(test_data)*4;
    assert_int_equal( cnt_first_alleles(test_data, num_cols), 93 );
    assert_int_equal( cnt_second_alleles(test_data, num_cols), 93 );
    uint8_t* buffer = (uint8_t*)malloc(sizeof(test_data) + sizeof(uint8_t));
    uint8_t* test_data_ma = buffer + 1;
    memcpy(test_data_ma, test_data, sizeof(test_data));
    assert_int_equal( cnt_first_alleles(test_data_ma, num_cols), 93 );
    assert_int_equal( cnt_second_alleles(test_data_ma, num_cols), 93 );
    free(buffer);
}

void
test_flip_alleles(void **state)
{
    UNUSED_PARAM(state);
    
    uint8_t test_data[] = {
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001,
        0b10110001
    };
    uint8_t correct_answer[] = {
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101,
        0b10001101
    };
    size_t num_cols = sizeof(test_data)*4;
    uint8_t* buffer = (uint8_t*)malloc(sizeof(test_data));
    memcpy(buffer, test_data, sizeof(test_data));
    flip_alleles(buffer, num_cols);
    assert_memory_equal( buffer, correct_answer, sizeof(test_data) );
    free(buffer);

    buffer = (uint8_t*)malloc(sizeof(test_data) + sizeof(uint8_t));
    uint8_t* buffer_ma = buffer + 1;
    memcpy(buffer_ma, test_data, sizeof(test_data));
    flip_alleles(buffer_ma, num_cols);
    assert_memory_equal( buffer_ma, correct_answer, sizeof(test_data) );
    free(buffer);
}

int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
    const UnitTest tests[] = {
        unit_test( test_cnt_alleles ),
        unit_test( test_flip_alleles ),
    };

    return run_tests( tests );
}
