/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __PLINKIO_PRIVATE_H__
#define __PLINKIO_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define UNUSED_PARAM(x) ((void)(x))

inline size_t pio_bits_msb_size(size_t x)
{
#if SIZE_MAX == UINTMAX_C(0xFFFF)
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    return x ^ (x >> 1);
#elif SIZE_MAX == UINTMAX_C(0xFFFFFFFF)
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x ^ (x >> 1);
#elif SIZE_MAX == UINTMAX_C(0xFFFFFFFFFFFFFFFF)
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    return x ^ (x >> 1);
#else
    size_t tmp = x;
    int bits = -1;
    while (tmp != 0)
    {
        tmp /= 2;
        bits++;
    }
    if (bits != -1) {
        return (size_t)1 << bits;
    } else {
        return 0;
    }
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* End of __PLINKIO_PRIVATE_H__ */
