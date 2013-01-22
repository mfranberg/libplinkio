/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __BED_HEADER_H__
#define __BED_HEADER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define BED_HEADER_MAX_SIZE 3

enum SnpOrder 
{
    /**
     * Means that when reading one row, you get one SNP for all
     * individuals (common).
     */
    BED_ONE_LOCUS_PER_ROW,

    /**
     * Means that when reading one row, you get all SNPs for
     * one individual.
     */
    BED_ONE_SAMPLE_PER_ROW,

    /**
     * Unknown order.
     */
    BED_UNKNOWN_ORDER
};

enum BedVersion
{
    /**
     * Old version, reading of these files might not work.
     */
    PIO_VERSION_PRE_099,

    /**
     * Version v0.99, no magic header but snp order in first byte.
     */
    PIO_VERSION_099,

    /**
     * Version v1.00, 16 bit magic header.
     */
    PIO_VERSION_100
};

struct bed_header_t
{
    /**
     * Order of the SNPs in the file.
     */
    enum SnpOrder snp_order;

    /**
     * Version of the file.
     */
    enum BedVersion version;

    /**
     * Number of loci.
     */
    size_t num_loci;

    /**
     * Number of samples.
     */
    size_t num_samples;
};

/**
 * Initializes and creates a bed header.
 *
 * By default the latest version is used, and the snps will be stored
 * accoding to ONE_LOCUS_PER_ROW.
 *
 * @param num_loci Number of loci.
 * @param num_samples Number of samples.
 *
 * @return The created bed header.
 */
struct bed_header_t bed_header_init(size_t num_loci, size_t num_samples);

/**
 * Initializes and creates a bed header from a 
 *
 * @param num_loci Number of loci.
 * @param num_samples Number of samples.
 * @param header_bytes Contains the header stored as bytes, it is at least
 *                     BED_HEADER_MAX_SIZE long.
 *
 * @return The created bed header.
 */
struct bed_header_t bed_header_init2(size_t num_loci, size_t num_samples, const unsigned char *header_bytes);

/**
 * Converts a packed header that is stored as an array of bytes into
 * a header structure.
 *
 * Note: The given header structure must be initialized.
 *
 * @param header The new header will be stored here.
 * @param header_bytes A packed header, assumed to be at least 
 *                     BED_HEADER_MAX_SIZE bytes long.
 */
void bed_header_from_bytes(struct bed_header_t *header, const unsigned char *header_bytes);

/**
 * Converts a header structure into a packed header.
 *
 * @param header Bed header.
 * @param header_bytes The packed header will be stored here, assumed to be at
 *                     least BED_HEADER_MAX_SIZE bytes long.
 * @param length Size of the packed header in bytes.
 *
 */
void bed_header_to_bytes(struct bed_header_t *header, unsigned char *header_bytes, int *length);

/**
 * Returns the number of rows stored in the file.
 *
 * @param header Bed header.
 *
 * @return The number of rows stored in the file.
 */
size_t bed_header_num_rows(struct bed_header_t *header);

/**
 * Returns the number of columns stored in the file.
 *
 * @param header Bed header.
 *
 * @return The number of columns stored in the file.
 */
size_t bed_header_num_cols(struct bed_header_t *header);

/**
 * Returns the length of a row in bytes for a bed file
 * with the given header.
 *
 * @param header Bed header.
 *
 * @return The number of bytes required to store a row of SNPs.
 */
size_t bed_header_row_size(struct bed_header_t *header);

/**
 * Returns the number of bytes occupied by the header,
 * i.e. the file offset to the data.
 *
 * @param header Bed header.
 *
 * @return the number of bytes occupied by the header.
 */
size_t bed_header_data_offset(struct bed_header_t *header);

/**
 * Returns the size of all rows in bytes.
 *
 * @param header Bed header.
 *
 * @return the size of all rows in bytes.
 */
size_t bed_header_data_size(struct bed_header_t *header);

/**
 * Returns the size of the file in bytes including the header.
 *
 * @param header Bed header.
 *
 * @return the size of the file in bytes including the header.
 */
size_t bed_header_file_size(struct bed_header_t *header);

/**
 * Returns the SNP order for the header.
 *
 * @param header Bed header.
 *
 * @return the SNP order for the header.
 */
enum SnpOrder bed_header_snp_order(struct bed_header_t *header);

/**
 * Transposes the given header.
 *
 * @param header Bed header.
 */
void bed_header_transpose(struct bed_header_t *header);

#ifdef __cplusplus
}
#endif

#endif
