/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __FAM_H__
#define __FAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <plinkio/utarray.h>
#include <plinkio/status.h>

/**
 * Sex of a sample.
 */
enum sex_t
{
    PIO_MALE,
    PIO_FEMALE,
    PIO_UNKNOWN
};

/**
 * Affection status.
 */
enum affection_t
{
    PIO_CONTROL = 0,
    PIO_CASE = 1,
    PIO_MISSING,
    PIO_CONTINUOUS 
};

/**
 * Data structure that contains the PLINK information about a sample (individual).
 */
struct pio_sample_t
{
    /**
     * An internal reference id, so that we can read them in order.
     */
    size_t pio_id;

    /**
     * Family identifier.
     */
    char *fid;

    /**
     * Plink individual identifier.
     */
    char *iid;

    /**
     * Plink individual identifier of father, 0 if none.
     */
    char *father_iid;

    /**
     * Plink individual identifier of mother, 0 if none.
     */
    char *mother_iid;

    /**
     * The sex of the individual.
     */
    enum sex_t sex;

    /**
     * Affection of the individuals, case, control or unkown. Control
     * is always 0 and case always 1.
     */
    enum affection_t affection;

    /**
     * A continuous phenotype of the individual.
     */
    float phenotype;
};

/**
 * Contains the information about a fam file. On opening the file it is
 * traversed and read into memory, each sample will have a record
 * in the sample array.
 */
struct pio_fam_file_t
{
    /**
     * Pointer to an opened and parse file.
     */
    FILE *fp;

    /**
     * List of additional information for each sample.
     */
    UT_array *sample;
};

/**
 * Opens the fam file at the given path and reads all individuals
 * into memory, and closes the file.
 *
 * @param fam_file Fam file.
 * @param path The location of the fam file.
 * 
 * @return Returns PIO_OK if the file could be read, PIO_ERROR otherwise.
 */
pio_status_t fam_open(struct pio_fam_file_t *fam_file, const char *path);

/**
 * Creates a fam file at the given path, and writes all
 * individuals to the file.
 *
 * @param fam_file Fam file.
 * @param path The location of the fam file.
 * @param samples List of samples.
 * @param num_samples Number of samples in the list.
 *
 * @return PIO_OK if the file could be created and written, PIO_ERROR otherwise.
 */
pio_status_t fam_create(struct pio_fam_file_t *fam_file, const char *path, struct pio_sample_t *samples, size_t num_samples);

/**
 * Returns the sample with the given pio_id.
 *
 * @param fam_file The fam file to get the sample from.
 * @param pio_id The pio id of the sample.
 *
 * @return the sample with the given pio_id.
 */
struct pio_sample_t * fam_get_sample(struct pio_fam_file_t *fam_file, size_t pio_id);

/**
 * Returns the number of samples that are stored in the given fam file.
 *
 * @param fam_file Fam file.
 * 
 * @return the number of samples that are stored in the fam file.
 */
size_t fam_num_samples(struct pio_fam_file_t *fam_file);

/**
 * Removes the read samples from memory.
 *
 * @param fam_file Fam file.
 */
void fam_close(struct pio_fam_file_t *fam_file);

#ifdef __cplusplus
}
#endif

#endif /* End of __FAM_H__ */
