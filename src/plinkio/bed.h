#ifndef __BED_H__
#define __BED_H__

enum SnpOrder 
{
    /**
     * Means that the SNPs are stored in columns in the file,
     * so when reading one row in the file you get one SNP
     * for all individuals (common).
     */
    SNPS_IN_COLUMNS,

    /**
     * Means that the SNPs are stored in rows in the file,
     * so when reading one row in the file you get the all 
     * SNPs for one individual (uncommon).
     */
    SNPS_IN_ROWS
};

enum BedVersion
{
    VERSION_PRE_099,
    VERSION_099,
    VERSION_100
};

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
};

#endif /* End of __BED_H__ */
