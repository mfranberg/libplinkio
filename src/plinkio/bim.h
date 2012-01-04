#ifndef __BIM_H__
#define __BIM_H__

#include <status.h>

/**
 * Converts a preprocessor constant to a string.
 *
 * Helper function of TO_STRING to convert the actual value of 
 * the constant and not the name. For example
 *
 * #define LENGTH 10
 * puts( STRINGIFY( LENGTH ) )
 *
 * would print "LENGTH". But adding a layer of indirection
 * through TO_STRING the preprocessor will convert
 *
 * TO_STRING( LENGTH ) to STRINGIFY( 10 )
 *
 * in the first pass, and finally resolve it to "10" in a second
 * pass.
 *
 * @param x A preprocessor constant.
 *
 * @return The constant as a string.
 */
#define STRINGIFY(x) #x

/**
 * Converts the value of preprocessor constant to a string.
 *
 * @param x A Preprocessor constant.
 * 
 * @return the constants value as a string.
 */
#define TO_STRING(x) STRINGIFY(x)

#define BIM_MAX_LOCUS_NAME 32

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
    char name[BIM_MAX_LOCUS_NAME];

    /**
     * Genetic position of the SNP.
     */
    unsigned long position;

    /**
     * Base pair position of the SNP.
     */
    unsigned long bp_position;

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
    size_t num_loci;

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

#endif /* End of __BIM_H__ */
