/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <plinkio/bed.h>
#include <plinkio/bed_header.h>
#include <plinkio/status.h>
#include <plinkio/snp_lookup.h>

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
    unsigned char header[ BED_HEADER_MAX_SIZE ];
    if( fread( header, 1, BED_HEADER_MAX_SIZE, bed_file->fp ) != BED_HEADER_MAX_SIZE )
    {
        return PIO_ERROR;
    }
    
    bed_header_from_bytes( &bed_file->header, header );

    fseek( bed_file->fp, bed_header_data_offset( &bed_file->header ), SEEK_SET );

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
unpack_snps(const snp_t *packed_snps, unsigned char *unpacked_snps, size_t num_cols)
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
 * Does the reverse of unpack_snps, and packs SNPs into bytes. See unpack_snps for
 * the detailed format.
 *
 * @param unpack_snps The unpacked SNPs.
 * @param packed_snps The packed SNPs.
 * @param num_cols The number of columns.
 */
void
pack_snps(const snp_t *unpacked_snps, unsigned char *packed_snps, size_t num_cols)
{
    int i;
    int packed_index;
    int position_in_byte;

    bzero( packed_snps, num_cols / 4 + 1 );
    for(i = 0; i < num_cols; i++)
    {
        /* Genotypes are stored backwards. */
        packed_index = i / 4;
        position_in_byte = (i % 4) * 2;
        packed_snps[ packed_index ] |= ( snp_to_bits[ unpacked_snps[ i ] ] << position_in_byte );
    }
}

/**
 * Transposes the given memory mapped file in place.
 *
 * @param rows Rows of the file.
 * @param num_rows The number of loci of the file to be transposed.
 * @param num_cols The number of samples of the file to be transposed.
 */
void
transpose_rows(const unsigned char *rows, size_t num_rows, size_t num_cols, FILE *output_file)
{
    int i, j;

    int num_bytes_per_row = ( num_cols + 3 ) / 4;
    int num_bytes_per_col = ( num_rows + 3 ) / 4;
    unsigned char *row_buffer = (unsigned char *) malloc( num_bytes_per_col );
    for(j = 0; j < num_cols; j++)
    {
        bzero( row_buffer, num_bytes_per_col );
        for(i = 0; i < num_rows; i++)
        {
            /* Index in the byte array */
            int from_index = i * num_bytes_per_row + j / 4;
            int to_index   =   i / 4;

            /* Index in the byte */
            int from_shift = ( j % 4 ) * 2;
            int to_shift = ( i % 4 ) * 2;

            /* Values to swap */
            int from_value = ( rows[ from_index ] >> from_shift ) & 0x3;
            
            row_buffer[ to_index ] |= ( from_value << to_shift );
        }

        fwrite( row_buffer, 1, num_bytes_per_col, output_file );
    }

    free( row_buffer );
}

/**
 * Transposes the given memory mapped file.
 *
 * @param mapped_file A memory mapped file.
 * @param num_loci The number of loci.
 * @param num_samples The number of samples.
 * @param output_path The transposed file will be stored here.
 *
 * @return Returns PIO_OK if the file could be transposed, PIO_ERROR otherwise.
 */
pio_status_t
transpose_file(const unsigned char *mapped_file, size_t num_loci, size_t num_samples, const char *output_path)
{
    struct bed_header_t header = bed_header_init2( num_loci, num_samples, mapped_file );
    size_t original_num_rows = bed_header_num_rows( &header );
    size_t original_num_cols = bed_header_num_cols( &header );
   
    FILE *output_file = fopen( output_path, "w" );
    if( output_file == NULL )
    {
        return PIO_ERROR;
    }
    
    /* Clear size of file, otherwise we might have trailing bytes */
    if( ftruncate( fileno( output_file ), 0 ) == - 1)
    {
        return PIO_ERROR;
    }
    
    /* Transpose and write header */
    unsigned char byte_header[ BED_HEADER_MAX_SIZE ];
    int byte_header_length = 0;
    bed_header_transpose( &header );
    bed_header_to_bytes( &header, byte_header, &byte_header_length );
    
    if( fwrite( byte_header, sizeof( unsigned char ), byte_header_length, output_file ) != byte_header_length )
    {
        fclose( output_file );
        return PIO_ERROR;
    }
    
    /* Transpose data */
    transpose_rows( mapped_file + bed_header_data_offset( &header ),
                    original_num_rows,
                    original_num_cols,
                    output_file );

    fclose( output_file );

    return PIO_OK;
}

pio_status_t
bed_open(struct pio_bed_file_t *bed_file, const char *path, size_t num_loci, size_t num_samples)
{
    size_t row_size_bytes;
    FILE *bed_fp;
   
    bzero( bed_file, sizeof( *bed_file ) );
    bed_fp = fopen( path, "r" );
    if( bed_fp == NULL )
    {
        return PIO_ERROR;
    }

    bed_file->fp = bed_fp;
    bed_file->header = bed_header_init( num_loci, num_samples );
    if( parse_header( bed_file ) != PIO_OK )
    {
        return PIO_ERROR;
    }
 
    row_size_bytes = bed_header_row_size( &bed_file->header ); 
    bed_file->read_buffer = ( snp_t * ) malloc( row_size_bytes );
    bed_file->cur_row = 0;

    return PIO_OK;
}

pio_status_t
bed_create(struct pio_bed_file_t *bed_file, const char *path, size_t num_samples)
{
    FILE *bed_fp;
    unsigned char header_bytes[3];
    int length;
    int row_size_bytes;
   
    bzero( bed_file, sizeof( *bed_file ) );
    bed_fp = fopen( path, "w" );
    if( bed_fp == NULL )
    {
        return PIO_ERROR;
    }

    bed_file->fp = bed_fp;
    bed_file->header = bed_header_init( 0, num_samples );
    bed_header_to_bytes( &bed_file->header, header_bytes, &length );
    if( fwrite( header_bytes, sizeof( unsigned char ), length, bed_fp ) <= 0 )
    {
        fclose( bed_fp );
        return PIO_ERROR;
    }
    
    row_size_bytes = bed_header_row_size( &bed_file->header ); 
    bed_file->read_buffer = ( snp_t * ) malloc( row_size_bytes );
    bed_file->cur_row = 0;

    return PIO_OK;
}

pio_status_t
bed_write_row(struct pio_bed_file_t *bed_file, snp_t *buffer)
{
    int row_size_bytes;
    int bytes_written;

    pack_snps( buffer, bed_file->read_buffer, bed_header_num_cols( &bed_file->header ) );
    row_size_bytes = bed_header_row_size( &bed_file->header );

    bytes_written = fwrite( bed_file->read_buffer, sizeof( unsigned char ), row_size_bytes, bed_file->fp );

    if( bytes_written > 0 )
    {
        bed_file->header.num_loci += 1;
        bed_file->cur_row += 1;

        return PIO_OK;
    }
    else
    {
        return PIO_ERROR;
    }
}

pio_status_t
bed_read_row(struct pio_bed_file_t *bed_file, snp_t *buffer)
{
    size_t row_size_bytes;
    size_t bytes_read;

    if( feof( bed_file->fp ) != 0 || bed_file->cur_row >= bed_header_num_rows( &bed_file->header ) )
    {
        return PIO_END;
    }

    row_size_bytes = bed_header_row_size( &bed_file->header );
    bytes_read = fread( bed_file->read_buffer, 
                        1,
                        row_size_bytes,
                        bed_file->fp );
    assert( bytes_read == row_size_bytes );

    if( bytes_read != row_size_bytes )
    {
        return PIO_ERROR;
    }

    unpack_snps( bed_file->read_buffer, buffer, bed_header_num_cols( &bed_file->header ) );
    bed_file->cur_row++;

    return PIO_OK;
}

size_t
bed_row_size(struct pio_bed_file_t *bed_file)
{
    return sizeof( snp_t ) * bed_header_num_cols( &bed_file->header );
}

size_t
bed_num_snps_per_row(struct pio_bed_file_t *bed_file)
{
    return bed_header_num_cols( &bed_file->header );
}

enum SnpOrder
bed_snp_order(struct pio_bed_file_t *bed_file)
{
    return bed_header_snp_order( &bed_file->header );
}

void
bed_reset_row(struct pio_bed_file_t *bed_file)
{
    fseek( bed_file->fp, bed_header_data_offset( &bed_file->header ), SEEK_SET );
    bed_file->cur_row = 0;
}

void
bed_close(struct pio_bed_file_t *bed_file)
{
    if( bed_file->fp == NULL )
    {
        return;
    }

    fclose( bed_file->fp );
    free( bed_file->read_buffer );
    bed_file->fp = NULL;
    bed_file->read_buffer = NULL;
}

pio_status_t
bed_transpose(const char *original_path, const char *transposed_path, size_t num_loci, size_t num_samples)
{
    /* Open original for mmap */ 
    int original_fd = open( original_path, O_RDONLY );
    if( original_fd == -1 )
    {
        return PIO_ERROR;
    }

    struct stat file_stats;
    if( fstat( original_fd, &file_stats ) == -1 )
    {
        return PIO_ERROR;
    }
   
    void *mapped_file = mmap( NULL,
                              file_stats.st_size,
                              PROT_READ,
                              MAP_FILE | MAP_PRIVATE,
                              original_fd,
                              0 );

    if( mapped_file == MAP_FAILED )
    {
        return PIO_ERROR;
    }

    /* Transpose */
    pio_status_t status = transpose_file( mapped_file, num_loci, num_samples, transposed_path );

    /* Release alloacted resources */
    munmap( mapped_file, file_stats.st_size );
    close( original_fd );
    
    return status;
}
