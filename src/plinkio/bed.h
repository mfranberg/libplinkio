#ifndef __BED_H__
#define __BED_H__

#include <stdio.h>

enum SnpOrder 
{
    /**
     * Means that when reading one row, you get one SNP for all
     * individuals (common).
     */
    ONE_LOCUS_PER_ROW,

    /**
     * Means that when reading one row, you get all SNPs for
     * one individual.
     */
    ONE_SAMPLE_PER_ROW,

    /**
     * Unknown order.
     */
    UNKNOWN
};

enum BedVersion
{
    VERSION_PRE_099,
    VERSION_099,
    VERSION_100
};

/**
 * Magic constants for v 1.00 format.
 */
#define BED_V100_MAGIC1 0x6c
#define BED_V100_MAGIC2 0x1b

/**
 * Mask for SNP order. 
 */
#define SNP_ORDER_BIT 0x80

/**
 * Number of bits used for each SNP, must be divisor
 * of 8.
 */
#define BITS_PER_SNP 2

/**
 * Masks out a SNP.
 */
#define SNP_MASK ( ( 1 << BITS_PER_SNP ) - 1 )

/**
 * Number of bits in a char.
 */
#define NUM_BITS_IN_CHAR ( sizeof( unsigned char ) * 8 )

/**
 * Number of SNPs packed in each char.
 */
#define SNPS_PER_CHAR ( NUM_BITS_IN_CHAR / BITS_PER_SNP )

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
    int num_cols;

    /**
     * The number of rows.
     */
    int num_rows;

    /**
     * Index of the current row.
     */
    int cur_row;
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
int bed_open(struct pio_bed_file_t *bed_file, const char *path, int num_loci, int num_samples);

/**
 * Reads a single row from the given bed_file.
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
int bed_read_row(struct pio_bed_file_t *bed_file, unsigned char *buffer);

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
