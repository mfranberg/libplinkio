/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __PIO_H__
#define __PIO_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <plinkio/bed.h>
#include <plinkio/bim.h>
#include <plinkio/fam.h>

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
pio_status_t pio_open(struct pio_file_t *plink_file, const char *plink_file_prefix);

/**
 * Opens the given plink file, which is specificed by separate paths
 * to the .bim, .bed and .fam files.
 *
 * @param plink_file Plink file.
 * @param bed_path Path to the .fam file.
 * @param bim_path Path to the .bim file.
 * @param fam_path Path to the .bed file.
 *
 * @return PIO_OK, if all files existed and could be read. PIO_ERROR otherwise.
 */
pio_status_t pio_open_ex(struct pio_file_t *plink_file, const char *fam_path, const char *bim_path, const char *bed_path);

/**
 * Returns a struct that contains information about the sample associated
 * with the given id. Note, any changes to this struct will be reflected if
 * you call pio_close with the save argument.
 *
 * @param plink_file Plink file.
 * @param pio_id Pio id of the sample. This is not the same as iid.
 *
 * @return The struct with the given id, or NULL if it does not exist.
 */
struct pio_sample_t * pio_get_sample(struct pio_file_t *plink_file, size_t pio_id);

/**
 * Returns the number of samples that are stored in the given plink file.
 *
 * @param plink_file Plink file.
 * 
 * @return the number of samples that are stored in the plink file.
 */
size_t pio_num_samples(struct pio_file_t *plink_file);

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
struct pio_locus_t * pio_get_locus(struct pio_file_t *plink_file, size_t pio_id);

/**
 * Returns the number of loci that are stored in the given plink file.
 *
 * @param plink_file Plink file.
 * 
 * @return the number of loci that are stored in the plink file.
 */
size_t pio_num_loci(struct pio_file_t *plink_file);

/**
 * Reads the next row from the bed file. Depending on the storage format,
 * this will return either a single SNP for all individuals, or all SNPs
 * for a single individual.
 *
 * The SNPs will be encoded as follows:
 * 0 - Homozygous major
 * 1 - Hetrozygous
 * 2 - Homozygous minor
 * 3 - Missing value
 *
 * @param plink_file Plink file.
 * @param buffer The row will be stored here. Must be able to hold at
 *               least pio_row_size bytes.
 *
 * @return PIO_OK if the row could be read, PIO_END if we are at the
 *         end of file, PIO_ERROR otherwise.
 */
pio_status_t pio_next_row(struct pio_file_t *plink_file, snp_t *buffer);

/**
 * Moves to the beginning of the file, so that the next call
 * of pio_next_row will return the first row.
 *
 * @param plink_file Plink file.
 */
void pio_reset_row(struct pio_file_t *plink_file);

/**
 * Returns the size of a row in bytes.
 *
 * @param plink_file Plink file.
 *
 * @return the size of a row in bytes.
 */
size_t pio_row_size(struct pio_file_t *plink_file);

/**
 * Determines whether a row represents one loci for
 * all individuals, or all loci for one individual.
 *
 * @param plink_file Plink file.
 *
 * @return 1 if one row contains one loci for
 *         all individuals, 0 otherwise.
 */
int pio_one_locus_per_row(struct pio_file_t *plink_file);

/**
 * Transposes the given file and writes it to another file.
 * 
 * @param plink_file_prefix Path to the plink files, without the extension.
 * @param transposed_file_prefix The transposed plink files will be stored at this path.
 *                               The .bim and .fam files will be copied.
 *
 * @return PIO_OK if the file could be transposed, PIO_ERROR otherwise.
 */
pio_status_t pio_transpose(const char *plink_file_prefix, const char *transposed_file_prefix);

/**
 * Closes all opened plink files. No changes are made.
 *
 * @param plink_file The file to close.
 */
void pio_close(struct pio_file_t *plink_file);

#ifdef __cplusplus
}
#endif

#endif /* __PIO_H__ */
