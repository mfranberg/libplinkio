#ifndef __BED_H__
#define __BED_H__

#include <stdio.h>

#include <status.h>

/**
 * Integral type used for storing a single SNP.
 */
typedef unsigned char snp_t;

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
     * Order of the SNPs in the file.
     */
    enum SnpOrder snp_order;

    /**
     * Version of the file.
     */
    enum BedVersion version;

    /**
     * Temporary buffer for each row.
     */
    unsigned char *read_buffer;

    /**
     * Number of columns.
     */
    size_t num_cols;

    /**
     * The number of rows.
     */
    size_t num_rows;

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
void bed_row_reset(struct pio_bed_file_t *bed_file);

/**
 * Closes the bed file.
 *
 * @param bed_file Bed file.
 */
void bed_close(struct pio_bed_file_t *bed_file);

#endif /* End of __BED_H__ */
