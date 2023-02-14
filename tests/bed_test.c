#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <stdint.h>

#include <cmockery.h>

#include <bed.h>
#include <bed_header.c>
#include <bed.c>
#include <file.c>
#include <utility.c>

/**
 * Mock functions.
 */

/**
 * Simple mock structure to do file read and write
 * in memory instead of accessing files. So when using
 * fread and its associates, it will actually read the
 * data here instead.
 */
struct
{
    /**
     * Data available to read.
     */
    unsigned char *data;

    /**
     * Length of data.
     */
    size_t data_length;

    /**
     * Current read position in the data.
     */
    size_t cur_pos;
} g_mock_data;

/**
 * Initializes the mock object with data.
 *
 * @param data The data.
 * @param data_length The length of the data. 
 */
void
mock_init(unsigned char *data, int data_length)
{
    g_mock_data.data = data;
    g_mock_data.data_length = data_length;
    g_mock_data.cur_pos = 0;
}

/**
 * Mocks feof by returning eof only if we are one passed the
 * last data element.
 */
int
mock_feof(FILE *stream)
{
    UNUSED_PARAM(stream);
    return g_mock_data.cur_pos >= g_mock_data.data_length;
}

/**
 * Mocks fread by reading from the g_mock_data instead.
 */
size_t
mock_fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
    UNUSED_PARAM(stream);
    size_t bytes_left = g_mock_data.data_length - g_mock_data.cur_pos;
    size_t bytes_to_read = size * nitems;
    if( bytes_to_read > bytes_left )
    {
        return 0;
    }

    memcpy( ptr, g_mock_data.data + g_mock_data.cur_pos, bytes_to_read );
    g_mock_data.cur_pos += bytes_to_read;

    return bytes_to_read;
}

/**
 * Mocks fopen, just returns a non-null pointer.
 */
FILE *
mock_fopen(const char *filename, const char *mode)
{
    UNUSED_PARAM(filename);
    UNUSED_PARAM(mode);
    return (FILE *)(uintptr_t)0xdeadbeef;
}

/**
 * Mocks fseek by moving in the in memory-data instead.
 */
int
mock_fseek(FILE *stream, long offset, int whence)
{
    UNUSED_PARAM(stream);
    if( whence == SEEK_SET )
    {
        if( (size_t)offset < g_mock_data.data_length )
        {
            g_mock_data.cur_pos = offset;
            return 0;
        }
        else
        {
            return 1;
        }
    }

    if( whence == SEEK_CUR )
    {
        if( (g_mock_data.cur_pos + offset)  < g_mock_data.data_length )
        {
            g_mock_data.cur_pos += offset;
            return 0;
        }
        else
        {
            return 1;
        }
    }

    // other whence options (e.i. SEEK_END) are not implemented.
    return 1;
}

/**
 * Mocks fclose by doing nothing.
 */
int mock_fclose(FILE *stream)
{
    UNUSED_PARAM(stream);
    return 0;
}

/**
 * Tests the parsing of a v. 1.00 header works.
 */
void
test_parse_header_v100(void **state)
{
    UNUSED_PARAM(state);
    struct pio_bed_file_t bed_file = {0};
    /**
     * v 1.00 file containing only a header.
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x01 };

    mock_init( file_data, 3 );
    assert_int_equal( parse_header( &bed_file ), PIO_OK );
    assert_int_equal( bed_file.header.version, PIO_VERSION_100 );
    assert_int_equal( bed_file.header.snp_order, BED_ONE_LOCUS_PER_ROW );
}

/**
 * Tests the parsing of a v. 0.99 header works.
 */
void
test_parse_header_v099(void **state)
{
    UNUSED_PARAM(state);
    struct pio_bed_file_t bed_file = {0};
    /**
     * v 0.99 file containing only a header.
     */
    unsigned char file_data[] = { 0, 0, 0 };

    mock_init( file_data, 3 );
    assert_int_equal( parse_header( &bed_file ), PIO_OK );
    assert_int_equal( bed_file.header.version, PIO_VERSION_099 );
    assert_int_equal( bed_file.header.snp_order, BED_ONE_SAMPLE_PER_ROW );
}

/**
 * Tests the parsing of a file where the header is too short.
 */
void
test_parse_header_bad(void **state)
{
    UNUSED_PARAM(state);
    struct pio_bed_file_t bed_file = {0};
    /**
     * File without a full header.
     */
    unsigned char file_data[] = { 1, 2 };

    mock_init( file_data, 2 );
    assert_int_equal( parse_header( &bed_file ), PIO_ERROR );
}

/**
 * Tests that open and close works.
 */
void
test_bed_open(void **state)
{
    UNUSED_PARAM(state);
    struct pio_bed_file_t bed_file = {0};
    /**
     * v 1.00 file containing only a header.
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x01 };

    mock_init( file_data, 3 );
    assert_int_equal( bed_open( &bed_file, "", 1, 2 ), PIO_OK );
    assert_int_equal( bed_file.header.version, PIO_VERSION_100 );
    assert_int_equal( bed_file.header.snp_order, BED_ONE_LOCUS_PER_ROW );
    assert_int_equal( bed_header_num_rows( &bed_file.header ), 1 );
    assert_int_equal( bed_header_num_cols( &bed_file.header ), 2 );
    
    bed_close( &bed_file );
}

/**
 * Tests that open and close works.
 */
void
test_bed_open2(void **state)
{
    UNUSED_PARAM(state);
    struct pio_bed_file_t bed_file = {0};
    /**
     * v 1.00 file containing only a header.
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x00 };

    mock_init( file_data, 3 );
    assert_int_equal( bed_open( &bed_file, "", 1, 2 ), PIO_OK );
    assert_int_equal( bed_file.header.version, PIO_VERSION_100 );
    assert_int_equal( bed_file.header.snp_order, BED_ONE_SAMPLE_PER_ROW );
    assert_int_equal( bed_header_num_rows( &bed_file.header ), 2 );
    assert_int_equal( bed_header_num_cols( &bed_file.header ), 1 );
    
    bed_close( &bed_file );
}

void
test_unpack_snps(void **state)
{
    UNUSED_PARAM(state);
    int i;
    /* packed_snps = [0, 1, 2, 3] */
    unsigned char packed_snps[] = { 0x78 };
    snp_t unpacked_snps[4];
    unpack_snps( packed_snps, unpacked_snps, 4 );
    
    for(i = 0; i < 4; i++)
    {
        assert_int_equal( unpacked_snps[ i ], i );
    } 
}

void
test_bed_row_size(void **state)
{
    UNUSED_PARAM(state);
    struct pio_bed_file_t bed_file;
    bed_file.header.snp_order = BED_ONE_LOCUS_PER_ROW;
    bed_file.header.num_loci = 1;
    bed_file.header.num_samples = 7;

    assert_int_equal( bed_row_size( &bed_file ), 2 );

    bed_file.header.num_loci = 1;
    bed_file.header.num_samples = 1;
    
    assert_int_equal( bed_row_size( &bed_file ), 1 );
}

void
test_bed_read_row(void **state)
{
    UNUSED_PARAM(state);
    int i, j;
    struct pio_bed_file_t bed_file;
    /**
     * v 1.00 file containing only a header, and two snp rows = 
     * [0, 1, 2, 3] = 0b01111000 = 0x78.
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x01, 0x78, 0x78 };

    mock_init( file_data, 5 );
    assert_int_equal( bed_open( &bed_file, "", 2, 4 ), PIO_OK );

    for(i = 0; i < 2; i++)
    {
        snp_t snps[4];
        assert_int_equal( bed_read_row( &bed_file, snps ), PIO_OK );

        for(j = 0; j < 4; j++)
        {
            assert_int_equal( snps[ j ], j );
        }
    }

    assert_int_equal( bed_read_row( &bed_file, 0 ), PIO_END );
    bed_close( &bed_file ); 
}

void
test_bed_skip_row(void **state)
{
    UNUSED_PARAM(state);
    int i, j;
    struct pio_bed_file_t bed_file;
    /**
     * v 1.00 file containing only a header, and two snp rows = 
     * [0, 1, 2, 3] = 0b01111000 = 0x78.
     * [3, 2, 1, 0] = 0b00101101 = 0x2d.
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x01, 0x78, 0x2d };

    mock_init( file_data, 5 );
    assert_int_equal( bed_open( &bed_file, "", 2, 4 ), PIO_OK );

    // skip first row
    assert_int_equal( bed_skip_row( &bed_file ), PIO_OK );

    for(i = 1; i < 2; i++)
    {
        snp_t snps[4];
        assert_int_equal( bed_read_row( &bed_file, snps ), PIO_OK );

        for(j = 0; j < 4; j++)
        {
            assert_int_equal( snps[ j ], 3 - j );
        }
    }

    assert_int_equal( bed_skip_row( &bed_file ), PIO_END );
    assert_int_equal( bed_read_row( &bed_file, 0 ), PIO_END );
    bed_close( &bed_file ); 
}

int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
    const UnitTest tests[] = {
        unit_test( test_parse_header_v100 ),
        unit_test( test_parse_header_v099 ),
        unit_test( test_parse_header_bad ),
        unit_test( test_bed_open ),
        unit_test( test_bed_open2 ),
        unit_test( test_unpack_snps ),
        unit_test( test_bed_read_row ),
        unit_test( test_bed_skip_row ),
    };

    return run_tests( tests );
}
