/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __BIM_H__
#define __BIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/utarray.h>
#include <plinkio/status.h>

/**
 * Data structure that contains the PLINK information about a locus (SNP).
 */
struct pio_locus_t
{
    /**
     * An internal reference id, so that we can read them in order.
     */
    size_t pio_id;

    /**
     * Chromosome number starting from 1.
     */
    unsigned char chromosome;

    /**
     * Name of the SNP.
     */
    char *name;

    /**
     * Genetic position of the SNP.
     */
    float position;

    /**
     * Base pair position of the SNP.
     */
    long long bp_position;

    /**
     * First allele.
     */
    char *allele1;

    /**
     * Second allele.
     */
    char *allele2;
};

/**
 * Contains the information about a bim file. On opening the file is
 * traversed and read into memory, each locus will have a record
 * in the locus array.
 */
struct pio_bim_file_t
{
    /**
     * File pointer of bim file
     */
    FILE *fp;

    /**
     * List of all locus in the file.
     */
    UT_array *locus;
};

/**
 * Opens the bim file at the given path and reads all loci
 * into memory, and closes the file.
 *
 * @param bim_file Bim file.
 * @param path The location of the bim file.
 * 
 * @return Returns PIO_OK if the file could be read, PIO_ERROR otherwise.
 */
pio_status_t bim_open(struct pio_bim_file_t *bim_file, const char *path);

/**
 * Returns the locus with the given pio_id.
 *
 * @param bim_file The bim file to get the locus from.
 * @param pio_id The pio id of the locus.
 *
 * @return the locus with the given pio_id.
 */
struct pio_locus_t * bim_get_locus(struct pio_bim_file_t *bim_file, size_t pio_id);

/**
 * Returns the number of loci that are stored in the given bim file.
 *
 * @param bim_file Bim file.
 * 
 * @return the number of loci that are stored in the bim file.
 */
size_t bim_num_loci(struct pio_bim_file_t *bim_file);

/**
 * Removes the read loci from memory.
 *
 * @param bim_file Bim file.
 */
void bim_close(struct pio_bim_file_t *bim_file);

#ifdef __cplusplus
}
#endif

#endif /* End of __BIM_H__ */
