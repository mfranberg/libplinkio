#include <stdlib.h>
#include <stdio.h>

#include <status.h>

#include <bed.h>
#include <snp_lookup.h>

/**
 * Creates mock versions of IO functions to allow unit testing.
 */
#ifdef UNIT_TESTING
    extern int mock_feof(FILE *stream);
    extern size_t mock_fread(void *ptr, size_t size, size_t nitems, FILE *stream);
    extern FILE *mock_fopen(const char *filename, const char *mode);
    extern int mock_fseek(FILE *stream, long offset, int whence);
    extern int mock_fclose(FILE *stream);

    #define feof mock_feof
    #define fread mock_fread
    #define fopen mock_fopen
    #define fseek mock_fseek
    #define fclose mock_fclose
#endif

/**
 * Magic constants for v 1.00 format.
 */
#define BED_V100_MAGIC1 0x6c
#define BED_V100_MAGIC2 0x1b

/**
 * Mask for SNP order. 
 */
#define BED_SNP_ORDER_BIT 0x80

/**
 * Number of bits used for each SNP, must be divisor
 * of 8.
 */
#define BED_BITS_PER_SNP 2

/**
 * Masks out a SNP.
 */
#define BED_SNP_MASK ( ( 1 << BED_BITS_PER_SNP ) - 1 )

/**
 * Number of bits in a char.
 */
#define BED_NUM_BITS_IN_CHAR ( sizeof( unsigned char ) * 8 )

/**
 * Number of SNPs packed in each char.
 */
#define BED_SNPS_PER_CHAR ( BED_NUM_BITS_IN_CHAR / BED_BITS_PER_SNP )

/**
 * Returns the SNP order encoded in
 * a byte.
 *
 * @param order SNP order encoded in a byte.
 *
 * @return ONE_LOCUS_PER_ROW if highest bit in order is set,
 *         ONE_SAMPLE_PER_ROW otherwise.
 */
int
get_snp_order(unsigned char order)
{
    if( order == BED_SNP_ORDER_BIT )
    {
        return PIO_ONE_LOCUS_PER_ROW;
    }
    else if( order == 0 )
    {
        return PIO_ONE_SAMPLE_PER_ROW;
    }
    else
    {
        return PIO_ONE_SAMPLE_PER_ROW;
    }
}

/**
 * Returns the file offset to the data for the 
 * given version.
 *
 * @param version Version of the bed file.
 *
 * @return File offset to the data section.
 */
int
get_data_offset(enum BedVersion version)
{
    if( version == PIO_VERSION_100 )
    {
        return 3;
    }
    else if( version == PIO_VERSION_099 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * Parses the header of a bed file and sets the file pointer
 * to the beginning of the data section.
 *
 * Note: Assumes that the file is at least 3 bytes long.
 *
 * @param bed_file Bed file.
 *
 * @return PIO_OK if the header could be read,
 *         PIO_ERROR otherwise.
 */
pio_status_t
parse_header(struct pio_bed_file_t *bed_file)
{
    unsigned char header[3];

    if( fread( header, 1, 3, bed_file->fp ) != 3 )
    {
        return PIO_ERROR;
    }
    
    if( header[ 0 ] == BED_V100_MAGIC1 && header[ 1 ] == BED_V100_MAGIC2 )
    {
        /* Version 1.00 */
        bed_file->version = PIO_VERSION_100;
        bed_file->snp_order = get_snp_order( header[ 2 ] );

    }
    else if( ( header[ 0 ] & ~BED_SNP_ORDER_BIT ) == 0 )
    {
        /* Version 0.99 (hopefully) */
        bed_file->version = PIO_VERSION_099;
        bed_file->snp_order = get_snp_order( header[ 1 ] );
    }
    else
    {
        /* Version < 0.99 */
        bed_file->version = PIO_VERSION_PRE_099;
        bed_file->snp_order = PIO_ONE_SAMPLE_PER_ROW;
    }

    fseek( bed_file->fp,get_data_offset( bed_file->version ), SEEK_SET );

    return PIO_OK;
}

/**
 * Take an unpacked array of SNPs where each SNP is packed in 2 bits,
 * and unpack into a byte array. This function assumes that the bits
 * are packed in the following manner:
 * - Each byte contains 4 SNPs
 * - The SNPs are read from right to left in each byte.
 * - The packed SNPs encoded as follows:
 *   * 00 is homozygous major 
 *   * 01 is missing value
 *   * 10 is hetrozygous
 *   * 11 is homozygous minor
 *
 * - The unpacked SNPs are encoded as follows:
 *   * 0 is homozygous major
 *   * 1 is hetrozygous 
 *   * 2 is homozygous minor
 *   * 3 is missing value
 *
 * @param packed_snps The packed SNPs.
 * @param unpacked_snps The unpacked SNPs.
 * @param num_cols The number of SNPs. 
 */
void
unpack_snps(const snp_t *packed_snps, unsigned char *unpacked_snps, int num_cols)
{
    int index;
    int packed_left;

    /* Unpack SNPs in pairs of 4. */
    int32_t *unpacked_snps_p = (int32_t *) unpacked_snps;
    int i;
    int packed_length = num_cols / 4;
    for(i = 0; i < packed_length; i++)
    { 
        *unpacked_snps_p = snp_lookup[ packed_snps[ i ] ].snp_block;
        unpacked_snps_p += 1;
    }

    /* Unpack the trailing SNPs */
    index = packed_length * 4;
    packed_left = num_cols % 4;
    for(i = 0; i < packed_left; i++)
    {
        unpacked_snps[ index + i ] = snp_lookup[ packed_snps[ packed_length ] ].snp_array[ i ];
    }
}

/**
 * Returns the row size in bytes of a packed row.
 *
 * @param num_cols The number of columns.
 *
 * @return the row size in bytes of a packed row.
 */
size_t
get_packed_row_size(int num_cols)
{
    return ( num_cols * BED_BITS_PER_SNP + BED_NUM_BITS_IN_CHAR - 1 ) / BED_NUM_BITS_IN_CHAR;
}

pio_status_t
bed_open(struct pio_bed_file_t *bed_file, const char *path, int num_loci, int num_samples)
{
    size_t row_size_bytes;
    FILE *bed_fp = fopen( path, "r" );
    if( bed_fp == NULL )
    {
        return PIO_ERROR;
    }

    bed_file->fp = bed_fp;
    if( parse_header( bed_file ) != PIO_OK )
    {
        return PIO_ERROR;
    }
    
    if( bed_file->snp_order == PIO_ONE_LOCUS_PER_ROW )
    {
        bed_file->num_cols = num_samples;
        bed_file->num_rows = num_loci;
    }
    else
    {
        bed_file->num_cols = num_loci;
        bed_file->num_rows = num_samples;
    }

    row_size_bytes = get_packed_row_size( bed_file->num_cols ); 
    bed_file->read_buffer = ( snp_t * ) malloc( row_size_bytes );
    bed_file->cur_row = 0;

    return PIO_OK;
}

pio_status_t
bed_read_row(struct pio_bed_file_t *bed_file, snp_t *buffer)
{
    size_t row_size_bytes;
    size_t bytes_read;

    if( feof( bed_file->fp ) != 0 || bed_file->cur_row >= bed_file->num_rows )
    {
        return PIO_END;
    }

    row_size_bytes = get_packed_row_size( bed_file->num_cols );
    bytes_read = fread( bed_file->read_buffer, 
                        1,
                        row_size_bytes,
                        bed_file->fp );

    if( bytes_read != row_size_bytes )
    {
        return PIO_ERROR;
    }

    unpack_snps( bed_file->read_buffer, buffer, bed_file->num_cols );
    bed_file->cur_row++;

    return PIO_OK;
}

size_t
bed_row_size(struct pio_bed_file_t *bed_file)
{
    return sizeof( snp_t ) * bed_file->num_cols;
}

enum SnpOrder
bed_snp_order(struct pio_bed_file_t *bed_file)
{
    return bed_file->snp_order;
}

void
bed_row_reset(struct pio_bed_file_t *bed_file)
{
    fseek( bed_file->fp, get_data_offset( bed_file->version ), SEEK_SET );
    bed_file->cur_row = 0;
}

void
bed_close(struct pio_bed_file_t *bed_file)
{
    free( bed_file->read_buffer );
    fclose( bed_file->fp );
}
