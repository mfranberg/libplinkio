#ifndef __BIM_H__
#define __BIM_H__

#define BIM_MAX_LOCUS_NAME 32

/**
 * Data structure that contains the PLINK information about a locus (SNP).
 */
struct pio_locus_t
{
    /**
     * An internal reference id, so that we can read them in order.
     */
    unsigned int pio_id;

    /**
     * Chromosome number starting from 1.
     */
    unsigned char chromosome;

    /**
     * Name of the SNP.
     */
    char name[BIM_MAX_LOCUS_NAME];

    /**
     * Genetic position of the SNP.
     */
    long position;

    /**
     * Base pair position of the SNP.
     */
    long bp_position;

    /**
     * Major allele.
     */
    char major;

    /**
     * Minor allele.
     */
    char minor;
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
     * The number of loci contained in the file.
     */
    unsigned int num_loci;

    /**
     * List of all locus in the file.
     */
    struct pio_locus_t *locus;
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
int bim_open(struct pio_bim_file_t *bim_file, const char *path);

/**
 * Returns the locus with the given pio_id.
 *
 * @param bim_file The bim file to get the locus from.
 * @param pio_id The pio id of the locus.
 *
 * @return the locus with the given pio_id.
 */
struct pio_locus_t * bim_get_locus(struct pio_bim_file_t *bim_file, unsigned int pio_id);

/**
 * Removes the read loci from memory.
 *
 * @param bim_file Bim file.
 */
void bim_close(struct pio_bim_file_t *bim_file);

#endif /* End of __BIM_H__ */
