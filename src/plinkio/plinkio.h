#ifndef __PIO_H__
#define __PIO_H__ 

#include <stdio.h>

#include <bed.h>
#include <bim.h>
#include <fam.h>

/**
 * Abstract structure for all the files that
 * are related to a PLINK file. On open all these
 * will be initialized.
 */
struct pio_file_t
{
    /**
     * Information from the bim file.
     */
    struct pio_bim_file_t bim_file;

    /**
     * Information from the fam file.
     */
    struct pio_fam_file_t fam_file;

    /**
     * Information from the bed file.
     */
    struct pio_bed_file_t bed_file;
};

/**
 * Opens the given plink file. Parses the fam and bim files.
 *
 * @param plink_file Plink file.
 * @param plink_file_prefix Path to the plink files, without the extension.
 *
 * @return PIO_OK, if all files existed and could be read. PIO_ERROR otherwise.
 */
int pio_open(struct pio_file_t *plink_file, const char *plink_file_prefix);

/**
 * Returns a struct that contains information about the sample associated
 * with the given id. Note, any changes to this struct will be reflected if
 * you call pio_close with the save argument.
 *
 * @param plink_file Plink file.
 * @param id Id of the sample, as returned by pio_next_row. This is not the same as
 *           iid.
 * @return The struct with the given id, or NULL if it does not exist.
 */
struct pio_sample_t * pio_get_sample(struct pio_file_t *plink_file, unsigned int id);

/**
 * Returns a struct that contains information about the locus associated
 * with the given id. Note, any changes to this struct will be reflected if
 * you call pio_close with the save argument.
 *
 * @param plink_file Plink file.
 * @param id Id of the locus, as returned by pio_next_row.
 *
 * @return The struct with the given id, or NULL if it does not exist.
 */
struct pio_locus_t * pio_get_locus(struct pio_file_t *plink_file, unsigned int id);

/**
 * Reads the next row from the bed file. Depending on the storage format,
 * this will return either a single SNP for all individuals, or all SNPs
 * for a single individual.
 *
 * @param plink_file Plink file.
 * @param buffer The row will be stored here. Must be able to hold at
 *               least pio_row_size bytes.
 *
 * @return PIO_OK if the row could be read, PIO_END if we are at the
 *         end of file, PIO_ERROR otherwise.
 */
unsigned int pio_next_row(struct pio_file_t *plink_file, unsigned char *buffer);

/**
 * Allocates a row buffer big enough to contain one row. This is a
 * convenience function, you can malloc the memory yourself by using
 * pio_row_size and malloc.
 * 
 * @param plink_file Plink file.
 * 
 * @return A pointer to the buffer.
 */
unsigned char *pio_allocate_row_buffer(struct pio_file_t *plink_file);

/**
 * Frees the given row buffer.
 *
 * @param buffer A buffer previously allocated by pio_allocate_row_buffer.
 */
void pio_free_row_buffer(unsigned char *buffer);

/**
 * Returns the size of a row in bytes.
 *
 * @param plink_file Plink file.
 *
 * @return the size of a row in bytes.
 */
size_t pio_row_size(struct pio_file_t *plink_file);

/**
 * Returns the row order for the SNPs.
 *
 * @param plink_file Plink file.
 *
 * @return the row order for the SNPs.
 */
enum SnpOrder pio_row_order(struct pio_file_t *plink_file);

/**
 * Closes all opened plink files. No changes are made.
 *
 * @param plink_file The file to close.
 */
void pio_close(struct pio_file_t *plink_file);

#endif /* __PIO_H__ */
