#ifndef __FAM_H__
#define __FAM_H__

#include <stdio.h>

#include <status.h>

/**
 * Sex of a sample.
 */
enum Sex
{
    MALE,
    FEMALE
};

/**
 * Outcome type.
 */
enum OutcomeType
{
    DISCRETE,
    CONTINUOUS
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
    unsigned int iid;

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
    enum Sex sex;

    /**
     * Type of outcome, continuous or case/control.
     */
    enum OutcomeType outcomeType;

    /**
     * The phenotype of the individual.
     */
    union
    {
        float as_float;
        int as_int;
    } phenotype;
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
 * Removes the read samples from memory.
 *
 * @param fam_file Fam file.
 */
void fam_close(struct pio_fam_file_t *fam_file);

#endif /* End of __FAM_H__ */
