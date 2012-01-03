#ifndef __FAM_H__
#define __FAM_H__

#include <stdio.h>

#include <status.h>

#define FAM_IID_MAX_LENGTH 11

/**
 * Sex of a sample.
 */
enum sex_t
{
    PIO_MALE,
    PIO_FEMALE
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
    unsigned int pio_id;

    /**
     * Family identifier.
     */
    unsigned int fid;

    /**
     * Plink individual identifier.
     */
    char iid[FAM_IID_MAX_LENGTH];

    /**
     * Plink individual identifier of father, 0 if none.
     */
    unsigned int father_iid;

    /**
     * Plink individual identifier of mother, 0 if none.
     */
    unsigned int mother_iid;

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
     * The number of samples.
     */
    unsigned int num_samples;

    /**
     * List of additional information for each sample.
     */
    struct pio_sample_t *sample;
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
int fam_open(struct pio_fam_file_t *fam_file, const char *path);

/**
 * Returns the sample with the given pio_id.
 *
 * @param fam_file The fam file to get the sample from.
 * @param pio_id The pio id of the sample.
 *
 * @return the sample with the given pio_id.
 */
struct pio_sample_t * fam_get_sample(struct pio_fam_file_t *fam_file, unsigned int pio_id);

/**
 * Returns the number of samples that are stored in the given fam file.
 *
 * @param fam_file Fam file.
 * 
 * @return the number of samples that are stored in the fam file.
 */
unsigned int fam_num_samples(struct pio_fam_file_t *fam_file);

/**
 * Removes the read samples from memory.
 *
 * @param fam_file Fam file.
 */
void fam_close(struct pio_fam_file_t *fam_file);

#endif /* End of __FAM_H__ */
