/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __SNP_LOOKUP_H__
#define __SNP_LOOKUP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_ENDIAN_H
#include <endian.h>
#elif HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#elif HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* End of __SNP_LOOKUP_H__ */
