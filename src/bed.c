#include <stdlib.h>
#include <stdio.h>

#include <status.h>

#include <bed.h>

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
    if( order == SNP_ORDER_BIT )
    {
        return ONE_LOCUS_PER_ROW;
    }
    else if( order == 0 )
    {
        return ONE_SAMPLE_PER_ROW;
    }
    else
    {
        return ONE_SAMPLE_PER_ROW;
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
    if( version == VERSION_100 )
    {
        return 3;
    }
    else if( version == VERSION_099 )
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
int
parse_header(struct pio_bed_file_t *bed_file)
{
    unsigned char header[3];

    if( fread( &header, 1, 3, bed_file->fp ) != 3 )
    {
        return PIO_ERROR;
    }
    
    if( header[ 0 ] == BED_V100_MAGIC1 && header[ 1 ] == BED_V100_MAGIC2 )
    {
        /* Version 1.00 */
        bed_file->version = VERSION_100;
        bed_file->snp_order = get_snp_order( header[ 2 ] );

    }
    else if( ( header[ 0 ] & ~SNP_ORDER_BIT ) == 0 )
    {
        /* Version 0.99 (hopefully) */
        bed_file->version = VERSION_099;
        bed_file->snp_order = get_snp_order( header[ 1 ] );
    }
    else
    {
        /* Version < 0.99 */
        bed_file->version = VERSION_PRE_099;
        bed_file->snp_order = ONE_SAMPLE_PER_ROW;
    }

    fseek( bed_file->fp,get_data_offset( bed_file->version ), SEEK_SET );

    return PIO_OK;
}

/**
 * Take an unpacked array of SNPs where each SNP is packed in 2 bits,
 * and unpack into a byte array.
 *
 * @param packed_snps The packed SNPs.
 * @param unpacked_snps The unpacked SNPs.
 * @param num_cols The number of SNPs. 
 */
void
unpack_snps(const unsigned char *packed_snps, unsigned char *unpacked_snps, int num_cols)
{
    int i;
    for(i = 0; i < num_cols; i++)
    {
        int pack_index = i / SNPS_PER_CHAR;
        int pack_shift = i % SNPS_PER_CHAR;
        unsigned char packed_snp = packed_snps[ pack_index ];
        
        // Start from the most significant (to the left) bits and go downwards
        int last_snp_shift = NUM_BITS_IN_CHAR - BITS_PER_SNP;
        int cur_snp_shift = last_snp_shift - pack_shift * BITS_PER_SNP;
        unsigned char snp = ( packed_snp >> cur_snp_shift ) & SNP_MASK;

        unpacked_snps[ i ] = snp;
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
    return ( num_cols * BITS_PER_SNP + NUM_BITS_IN_CHAR - 1 ) / NUM_BITS_IN_CHAR;
}

int
bed_open(struct pio_bed_file_t *bed_file, const char *path, int num_cols)
{
    size_t row_size_bytes;
    FILE *bed_fp = fopen( path, "r" );
    if( bed_fp == NULL )
    {
        return PIO_ERROR;
    }

    bed_file->fp = bed_fp;
    bed_file->num_cols = num_cols;
    // And here
    
    row_size_bytes = get_packed_row_size( bed_file->num_cols ); 
    bed_file->read_buffer = ( unsigned char * ) malloc( row_size_bytes );

    return parse_header( bed_file );
}

int
bed_read_row(struct pio_bed_file_t *bed_file, unsigned char *buffer)
{
    size_t row_size_bytes;
    size_t bytes_read;

    if( feof( bed_file->fp ) != 0 )
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
        if( feof( bed_file->fp ) != 0 )
        {
            return PIO_END;
        }
        else
        {
            return PIO_ERROR;
        }
    }

    unpack_snps( bed_file->read_buffer, buffer, bed_file->num_cols );

    return PIO_OK;
}

size_t
bed_row_size(struct pio_bed_file_t *bed_file)
{
    return sizeof( unsigned char ) * bed_file->num_cols;
}

void
bed_row_reset(struct pio_bed_file_t *bed_file)
{
    fseek( bed_file->fp, get_data_offset( bed_file->version ), SEEK_SET );
}

void
bed_close(struct pio_bed_file_t *bed_file)
{
    free( bed_file->read_buffer );
    fclose( bed_file->fp );
}
