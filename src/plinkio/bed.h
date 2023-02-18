/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __BED_H__
#define __BED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <plinkio/status.h>
#include <plinkio/bed_header.h>

/**
 * Integral type used for storing a single SNP.
 */
typedef unsigned char snp_t;

/**
 * Contains the information about a bed file. On opening the file
 * header is read and the information stored in this structure.
 */
struct pio_bed_file_t
{
    /**
     * File pointer of bed file
     */
    FILE *fp;

    /**
     * Header of the bed file.
     */
    struct bed_header_t header;

    /**
     * Temporary buffer for each row.
     */
    unsigned char *read_buffer;

    /**
     * Index of the current row.
     */
    size_t cur_row;
};

/**
 * Opens the bed file and reads the header, the data is
 * not read until explicitly asking for it.
 *
 * @param bed_file Bed file.
 * @param path Path to the bed file.
 * @param num_loci The number loci.
 * @param num_samples The number of samples.
 *
 * @return PIO_OK if the file could be opened, PIO_ERROR otherwise.
 */
pio_status_t bed_open(struct pio_bed_file_t *bed_file, const char *path, size_t num_loci, size_t num_samples);

/**
 * Creates a bed file.
 *
 * @param bed_file Bed file.
 * @param path Path to the bed file.
 * @param num_samples The number of samples that the .bed file will include.
 *
 * @return PIO_OK if the file could be created, PIO_ERROR otherwise.
 */
pio_status_t bed_create(struct pio_bed_file_t *bed_file, const char *path, size_t num_samples);

/**
 * Writes a single row of samples to the bed file, assuming that the size of
 * the buffer is at least as big as specified when created.
 *
 * @param bed_file Bed file.
 * @param buffer List of SNPs to write to file.
 */
pio_status_t bed_write_row(struct pio_bed_file_t *bed_file, const snp_t *buffer);

/**
 * Reads a single row from the given bed_file. Each element in the buffer
 * will contain a SNP. The SNP will be encoded as follows:
 * 0 - Homozygous major
 * 1 - Hetrozygous
 * 2 - Homozygous minor
 * 3 - Missing value
 *
 * @param bed_file Bed file.
 * @param buffer The buffer to read into. It is asssumed that it
 *               is big enough, e.g.:
 *               SNPS_IN_ROWS: Number of individuals
 *               SNPS_IN_COLUMNS: Number of loci.
 *
 * @return PIO_OK if a row could be read,
 *         PIO_END if there are no more rows,
 *         PIO_ERROR otherwise.
 */
pio_status_t bed_read_row(struct pio_bed_file_t *bed_file, snp_t *buffer);

/**
 * Skips a single row from the given bed_file.
 *
 * @param bed_file Bed file.
 *
 * @return PIO_OK if a row could be skipped,
 *         PIO_END if there are no more rows,
 *         PIO_ERROR otherwise.
 */
pio_status_t bed_skip_row(struct pio_bed_file_t *bed_file);

/**
 * Returns the number of bytes required to store a row from
 * the given bed file.
 *
 * @param bed_file Bed file.
 *
 * @return The number of bytes required to store a row from
 * the given bed file.
 */
size_t bed_row_size(struct pio_bed_file_t *bed_file);

/**
 * Returns the number of snps stored in a row for the
 * given bed file.
 *
 * @param bed_file Bed file. 
 *
 * @return The number of snps stroed in a row.
 */
size_t bed_num_snps_per_row(struct pio_bed_file_t *bed_file);

/**
 * Returns the SNP order for the given bed file.
 *
 * @param bed_file Bed file.
 * 
 * @return the SNP order for the given bed file.
 */
enum SnpOrder bed_snp_order(struct pio_bed_file_t *bed_file);

/**
 * Restarts the reading at the first row.
 *
 * @param bed_file Bed file.
 */
void bed_reset_row(struct pio_bed_file_t *bed_file);

/**
 * Closes the bed file.
 *
 * @param bed_file Bed file.
 */
void bed_close(struct pio_bed_file_t *bed_file);

/**
 * Transposes the given file to the given output file.
 *
 * @param original_path The file to transpose.
 * @param transposed_path The file where the transposed data
 *        will be stored.
 * @param num_loci The number of loci.
 * @param num_samples The number of samples.
 *
 */
pio_status_t bed_transpose(const char *original_path, const char *transposed_path, size_t num_loci, size_t num_samples);

#ifdef __cplusplus
}
#endif

#endif /* End of __BED_H__ */
