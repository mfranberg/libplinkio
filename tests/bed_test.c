#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cmockery.h>

#include <bed.h>
#include <bed.c>

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
    int data_length;

    /**
     * Current read position in the data.
     */
    int cur_pos;
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
    return g_mock_data.cur_pos >= g_mock_data.data_length;
}

/**
 * Mocks fread by reading from the g_mock_data instead.
 */
size_t
mock_fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
    int bytes_left = g_mock_data.data_length - g_mock_data.cur_pos;
    int bytes_to_read = size * nitems;
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
    return (FILE *) 0xdeadbeef;
}

/**
 * Mocks fseek by moving in the in memory-data instead.
 */
int
mock_fseek(FILE *stream, long offset, int whence)
{
    if( offset < g_mock_data.data_length )
    {
        g_mock_data.cur_pos = offset;
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
 * Mocks fclose by doing nothing.
 */
int mock_fclose(FILE *stream)
{
    return 0;
}

/**
 * Tests the parsing of a v. 1.00 header works.
 */
void
test_parse_header_v100(void **state)
{
    struct pio_bed_file_t bed_file;
    /**
     * v 1.00 file containing only a header.
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x80 };

    mock_init( file_data, 3 );
    assert_int_equal( parse_header( &bed_file ), PIO_OK );
    assert_int_equal( bed_file.version, VERSION_100 );
    assert_int_equal( bed_file.snp_order, ONE_LOCUS_PER_ROW );
}

/**
 * Tests the parsing of a v. 0.99 header works.
 */
void
test_parse_header_v099(void **state)
{
    struct pio_bed_file_t bed_file;
    /**
     * v 0.99 file containing only a header.
     */
    unsigned char file_data[] = { 0, 0, 0 };

    mock_init( file_data, 3 );
    assert_int_equal( parse_header( &bed_file ), PIO_OK );
    assert_int_equal( bed_file.version, VERSION_099 );
    assert_int_equal( bed_file.snp_order, ONE_SAMPLE_PER_ROW );
}

/**
 * Tests the parsing of a file where the header is too short.
 */
void
test_parse_header_bad(void **state)
{
    struct pio_bed_file_t bed_file;
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
    struct pio_bed_file_t bed_file;
    /**
     * v 1.00 file containing only a header.
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x80 };

    mock_init( file_data, 3 );
    assert_int_equal( bed_open( &bed_file, "", 1 ), PIO_OK );
    assert_int_equal( bed_file.version, VERSION_100 );
    assert_int_equal( bed_file.snp_order, ONE_LOCUS_PER_ROW );
    
    bed_close( &bed_file );
}

void
test_unpack_snps(void **state)
{
    int i;
    /* packed_snps = [0, 1, 2, 3] */
    unsigned char packed_snps[] = { 0x1b };
    unsigned char unpacked_snps[4];
    unpack_snps( packed_snps, unpacked_snps, 4 );
    
    for(i = 0; i < 4; i++)
    {
        assert_int_equal( unpacked_snps[ i ], i );
    } 
}

void
test_bed_read_row(void **state)
{
    int i, j;
    struct pio_bed_file_t bed_file;
    /**
     * v 1.00 file containing only a header, and two snp rows = [0, 1, 2, 3].
     */
    unsigned char file_data[] = { BED_V100_MAGIC1, BED_V100_MAGIC2, 0x80, 0x1b, 0x1b };

    mock_init( file_data, 5 );
    assert_int_equal( bed_open( &bed_file, "", 4 ), PIO_OK );

    for(i = 0; i < 2; i++)
    {
        unsigned char snps[4];
        assert_int_equal( bed_read_row( &bed_file, snps ), PIO_OK );

        for(j = 0; j < 4; j++)
        {
            assert_int_equal( snps[ j ], j );
        }
    }

    assert_int_equal( bed_read_row( &bed_file, 0 ), PIO_END );
    bed_close( &bed_file ); 
}

int main(int argc, char* argv[])
{
    const UnitTest tests[] = {
        unit_test( test_parse_header_v100 ),
        unit_test( test_parse_header_v099 ),
        unit_test( test_parse_header_bad ),
        unit_test( test_bed_open ),
        unit_test( test_unpack_snps ),
        unit_test( test_bed_read_row ),
    };

    return run_tests( tests );
}
