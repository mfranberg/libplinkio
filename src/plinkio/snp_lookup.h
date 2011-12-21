#ifndef __SNP_LOOKUP_H__
#define __SNP_LOOKUP_H__

#include <machine/endian.h>

/**
 * This files contains a lookup table that maps
 * SNPs packed in a single byte into an array of
 * four bytes.
 */
union snp_lookup_t
{
    /**
     * Accessible as an array.
     */
    unsigned char snp_array[4];

    /**
     * Accessible as a block of bytes.
     */
    int32_t snp_block;
};

#if __BYTE_ORDER == __LITTLE_ENDIAN
#include "snp_lookup_little.h"
#else
#include "snp_lookup_big.h"
#endif /* End test endianess */

#endif /* End of __SNP_LOOKUP_H__ */
