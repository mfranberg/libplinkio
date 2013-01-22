/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __BIM_PARSE_H__
#define __BIM_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/status.h>

/**
 * Parses the loci and points the given locus array to a
 * the memory that contains them, and writes back the number
 * of loci.
 *
 * @param bim_file Bim file.
 *
 * @return PIO_OK if the loci could be parsed, PIO_ERROR otherwise.
 */
pio_status_t parse_loci(FILE *bim_fp, UT_array *locus);

#ifdef __cplusplus
}
#endif

#endif /* End of __BIM_PARSE_H__ */
