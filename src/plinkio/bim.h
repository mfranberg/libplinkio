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
    unsigned int num_locus;

    /**
     * List of all locus in the file.
     */
    struct pio_locus_t *locus;
};

#endif /* End of __BIM_H__ */
