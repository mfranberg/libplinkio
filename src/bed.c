/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <plinkio/bed.h>
#include <plinkio/bed_header.h>
#include <plinkio/status.h>
#include <plinkio/snp_lookup.h>

#include "private/utility.h"
#include "private/bed.h"

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

    fseek( bed_file->fp, (long)bed_header_data_offset( &bed_file->header ), SEEK_SET );

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
unpack_snps(const snp_t *packed_snps, uint8_t *unpacked_snps, size_t num_cols)
{
    size_t packed_length = num_cols / 4;
    /* Unpack SNPs in pairs of 4. */
    uint8_t* p = unpacked_snps;
    if (((uintptr_t)unpacked_snps & 0b11) == 0) {
        // 4 bytes aligned
        for (size_t i = 0; i < packed_length; i++) {
            *((uint32_t*)p) = *((uint32_t*)(snp_lookup[ packed_snps[ i ] ].snp_array));
            p += 4;
        }
    } else if (((uintptr_t)unpacked_snps & 0b1) == 0) {
        // 2 byte aligned
        for (size_t i = 0; i < packed_length; i++) {
            *((uint16_t*)p) = *((uint16_t*)(snp_lookup[ packed_snps[ i ] ].snp_array));
            *((uint16_t*)(p + 2)) = *((uint16_t*)(snp_lookup[ packed_snps[ i ] ].snp_array + 2));
            p += 4;
        }
    } else {
        // Unaligned
        for (size_t i = 0; i < packed_length; i++) {
            *(p) = *(snp_lookup[ packed_snps[ i ] ].snp_array);
            *(p + 1) = *(snp_lookup[ packed_snps[ i ] ].snp_array + 1);
            *(p + 2) = *(snp_lookup[ packed_snps[ i ] ].snp_array + 2);
            *(p + 3) = *(snp_lookup[ packed_snps[ i ] ].snp_array + 3);
            p += 4;
        }
    }

    /* Unpack the trailing SNPs */
    size_t index = packed_length * 4;
    size_t packed_left = num_cols % 4;
    for(size_t i = 0; i < packed_left; i++) {
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
    memset( packed_snps, 0, (num_cols + 3) / 4 );
    for(size_t i = 0; i < num_cols; i++)
    {
        /* Genotypes are stored backwards. */
        size_t packed_index = i / 4;
        size_t position_in_byte = (i % 4) * 2;
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
    size_t num_bytes_per_row = ( num_cols + 3 ) / 4;
    size_t num_bytes_per_col = ( num_rows + 3 ) / 4;
    unsigned char *row_buffer = (unsigned char *) malloc( num_bytes_per_col );
    for(size_t j = 0; j < num_cols; j++)
    {
        memset( row_buffer, 0, num_bytes_per_col );
        for(size_t i = 0; i < num_rows; i++)
        {
            /* Index in the byte array */
            size_t from_index = i * num_bytes_per_row + j / 4;
            size_t to_index   =   i / 4;

            /* Index in the byte */
            size_t from_shift = ( j % 4 ) * 2;
            size_t to_shift = ( i % 4 ) * 2;

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
 *
 * @deprecated This function is left only for compatibility.
 */
pio_status_t
transpose_file(const unsigned char *mapped_file, size_t num_loci, size_t num_samples, const char *output_path)
{
    struct bed_header_t header = bed_header_init2( num_loci, num_samples, mapped_file );
    size_t original_num_rows = bed_header_num_rows( &header );
    size_t original_num_cols = bed_header_num_cols( &header );
    unsigned char byte_header[ BED_HEADER_MAX_SIZE ];
    size_t byte_header_length = 0;
   
    int transposed_fd = -1;
    FILE* output_file = fopen( output_path, "wb" );
    if( output_file == NULL ) goto error;

    transposed_fd = fileno(output_file);
    if( transposed_fd < 0 ) goto error;

    /* Clear size of file, otherwise we might have trailing bytes */
    if(libplinkio_ftruncate_(transposed_fd, 0) == -1) goto error;
    
    /* Transpose and write header */
    bed_header_transpose( &header );
    bed_header_to_bytes( &header, byte_header, &byte_header_length );
    
    if(
        fwrite(
            byte_header, sizeof( unsigned char ), byte_header_length, output_file
        ) != byte_header_length
    ) goto error;
    
    /* Transpose data */
    transpose_rows( mapped_file + bed_header_data_offset( &header ),
                    original_num_rows,
                    original_num_cols,
                    output_file );

    fclose( output_file );

    return PIO_OK;

error:
    if (output_file != NULL) fclose(output_file);
    return PIO_ERROR;
}

pio_status_t
bed_open(struct pio_bed_file_t *bed_file, const char *path, size_t num_loci, size_t num_samples)
{
    size_t row_size_bytes;
    FILE *bed_fp;
   
    memset( bed_file, 0, sizeof( *bed_file ) );
    bed_fp = fopen( path, "rb" );
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
    size_t length;
    size_t row_size_bytes;
   
    memset( bed_file, 0, sizeof( *bed_file ) );
    bed_fp = fopen( path, "wb" );
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
bed_write_row(struct pio_bed_file_t *bed_file, const snp_t *buffer)
{
    pack_snps( buffer, bed_file->read_buffer, bed_header_num_cols( &bed_file->header ) );
    size_t row_size_bytes = bed_header_row_size( &bed_file->header );

    size_t bytes_written = fwrite( bed_file->read_buffer, sizeof( unsigned char ), row_size_bytes, bed_file->fp );

    if( bytes_written > 0 )
    {
        if( bed_file->header.snp_order == BED_ONE_LOCUS_PER_ROW ) {
            bed_file->header.num_loci += 1;
        } else {
            bed_file->header.num_samples += 1;
        }
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

pio_status_t
bed_skip_row(struct pio_bed_file_t *bed_file)
{
    size_t row_size_bytes;

    if( feof( bed_file->fp ) != 0 || bed_file->cur_row >= bed_header_num_rows( &bed_file->header ) )
    {
        return PIO_END;
    }

    row_size_bytes = bed_header_row_size( &bed_file->header );
    if( fseek( bed_file->fp, (long)row_size_bytes, SEEK_CUR ) ) {
        return PIO_ERROR;
    }

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
    fseek( bed_file->fp, (long)bed_header_data_offset( &bed_file->header ), SEEK_SET );
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

/**
 * Transposes the given file descriptor.
 *
 * @param original_fd Input file descriptor.
 * @param transposed_fd Output file descriptor.
 * @param num_loci The number of loci.
 * @param num_samples The number of samples.
 *
 * @return Returns PIO_OK if the file could be transposed, PIO_ERROR otherwise.
 */
pio_status_t
libplinkio_bed_transpose_fd_(const int original_fd, const int transposed_fd, size_t num_loci, size_t num_samples)
{
    unsigned char* mapped_file = NULL;
    int dup_fd = -1;
    FILE* output_file = NULL;
    libplinkio_mmap_state_private_t mmap_stats = { 0 };

    if(original_fd < 0) goto error;

    mapped_file = (unsigned char*)libplinkio_mmap_(original_fd, LIBPLINKIO_MMAP_READONLY_, &mmap_stats);
    if (mapped_file == NULL) goto error;
    
    /* Clear size of file, otherwise we might have trailing bytes */
    if(libplinkio_ftruncate_(transposed_fd, 0) == -1) goto error;

    // Open stream
    dup_fd = dup(transposed_fd);
    if (dup_fd == -1) goto error;
    output_file = fdopen(dup_fd, "wb");
    if (output_file == NULL) goto error;
    dup_fd = -1;

    struct bed_header_t header = bed_header_init2( num_loci, num_samples, mapped_file );
    size_t original_num_rows = bed_header_num_rows( &header );
    size_t original_num_cols = bed_header_num_cols( &header );
    unsigned char byte_header[ BED_HEADER_MAX_SIZE ];
    size_t byte_header_length = 0;

    /* Transpose and write header */
    bed_header_transpose( &header );
    bed_header_to_bytes( &header, byte_header, &byte_header_length );
    
    if( fwrite( byte_header, sizeof( unsigned char ), byte_header_length, output_file ) != byte_header_length ) goto error;
    
    /* Transpose data */
    transpose_rows(
        mapped_file + byte_header_length,
        original_num_rows,
        original_num_cols,
        output_file
    );

    /* Release alloacted resources */
    if (libplinkio_munmap_(mapped_file, &mmap_stats) != 0) {
        mapped_file = NULL;
        goto error;
    }
    fclose(output_file);

    return PIO_OK;

error:
    if (dup_fd != -1) close(dup_fd);
    if (output_file != NULL) fclose(output_file);
    if (mapped_file != NULL) libplinkio_munmap_(mapped_file, &mmap_stats);
    return PIO_ERROR;
}

pio_status_t
bed_transpose(const char *original_path, const char *transposed_path, size_t num_loci, size_t num_samples)
{
    int original_fd = -1;
    int transposed_fd = -1;

#ifdef _WIN32
    original_fd = open( original_path, O_RDONLY | O_BINARY );
#else
    original_fd = open( original_path, O_RDONLY );
#endif
    if( original_fd == -1 ) goto error;

#ifdef _WIN32
    transposed_fd = open( transposed_path, O_WRONLY | O_BINARY );
#else
    transposed_fd = open( transposed_path, O_WRONLY );
#endif
    if( transposed_fd == -1 ) goto error;

    /* Transpose */
    if (libplinkio_bed_transpose_fd_(original_fd, transposed_fd, num_loci, num_samples) != PIO_OK) goto error;

    close(transposed_fd);
    close(original_fd);
    return PIO_OK;

error:
    if (transposed_fd != -1) close(transposed_fd);
    if (original_fd != -1) close(original_fd);
    return PIO_ERROR;
}

pio_status_t
libplinkio_bed_transpose_pio_bed_file_(struct pio_bed_file_t *bed_file, const char *transposed_path, size_t num_loci, size_t num_samples, _Bool is_tmp)
{
    int original_fd = -1;
    int transposed_fd = -1;
    FILE* transposed_fp = NULL;
    size_t row_size_bytes = 0;

    if( bed_file->fp == NULL ) goto error;
    rewind(bed_file->fp);
    fflush(bed_file->fp);
    original_fd = fileno(bed_file->fp);
    if (original_fd == -1) goto error;

    if (is_tmp) {
        transposed_fd = libplinkio_tmp_open_(transposed_path, strlen(transposed_path));
    } else {
#ifdef _WIN32
        transposed_fd = open( transposed_path, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
#else
        transposed_fd = open( transposed_path, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE );
#endif
    }
    if( transposed_fd == -1 ) goto error;

    /* Transpose */
    if (libplinkio_bed_transpose_fd_(original_fd, transposed_fd, num_loci, num_samples) != PIO_OK) goto error;
    if (lseek(transposed_fd, 0, SEEK_SET) == -1) goto error; 

    bed_file->header = bed_header_init( num_loci, num_samples );
    row_size_bytes = bed_header_row_size(&bed_file->header);

    // Write new bed_file
    free(bed_file->read_buffer);
    bed_file->read_buffer = (unsigned char*)malloc(row_size_bytes);
    if (bed_file->read_buffer == NULL) goto error;

    transposed_fp = fdopen(transposed_fd, "w+b");
    if (transposed_fp == NULL) goto error;
    transposed_fd = -1;
    fclose(bed_file->fp);
    bed_file->fp = transposed_fp;
    transposed_fp = NULL;

    bed_file->cur_row = 0;

    return PIO_OK;

error:
    if (bed_file->read_buffer != NULL) free(bed_file->read_buffer);
    *bed_file = (struct pio_bed_file_t){0};
    if (transposed_fp != NULL) fclose(transposed_fp);
    if (transposed_fd != -1) close(transposed_fd);
    return PIO_ERROR;
}

pio_status_t
libplinkio_bed_tmp_transposed_create_(struct pio_bed_file_t* bed_file, const char* path_prefix, size_t num_loci)
{
    unsigned char header_bytes[BED_HEADER_MAX_SIZE];
    size_t length;
    size_t row_size_bytes;

    int fd = -1;
    FILE* fp = NULL;
   
    *bed_file = (struct pio_bed_file_t){ 0 };
    size_t path_prefix_length = strlen(path_prefix);

    fd = libplinkio_tmp_open_(path_prefix, path_prefix_length);
    if (fd == -1) goto error;
    fp = fdopen(fd, "w+b");
    if(fp == NULL) goto error;
    fd = -1;

    bed_file->fp = fp;
    bed_file->header = bed_header_init( num_loci, 0 );
    bed_header_transpose( &bed_file->header );
    bed_header_to_bytes( &bed_file->header, header_bytes, &length );
    if(
        fwrite( header_bytes, sizeof( unsigned char ), length, fp ) <= 0
    ) goto error;
  
    row_size_bytes = bed_header_row_size( &bed_file->header );
    bed_file->read_buffer = (snp_t *)calloc(row_size_bytes, sizeof(snp_t));
    if (bed_file->read_buffer == NULL) goto error;
    bed_file->cur_row = 0;

    return PIO_OK;

error:
    if (fp != NULL) fclose(fp);
    if (fd != -1) close(fd);
    free(bed_file->read_buffer);
    *bed_file = (struct pio_bed_file_t){ 0 };
    return PIO_ERROR;
}

pio_status_t libplinkio_change_bed_read_only_ (struct pio_bed_file_t* bed_file) {
    fflush(bed_file->fp);
    int fd = libplinkio_change_mode_and_open_(fileno(bed_file->fp), O_RDONLY);
    if (fd < 0) goto error;
    fclose(bed_file->fp);
    bed_file->fp = fdopen(fd, "rb");
    if (bed_file->fp == NULL) goto error;
    return PIO_OK;
error:
    if (fd >= 0) close(fd);
    return PIO_ERROR;
}