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
 * @param bim_fp Bim file.
 * @param locus The parsed loci will be stored here.
 *
 * @return PIO_OK if the loci could be parsed, PIO_ERROR otherwise.
 */
pio_status_t parse_loci(FILE *bim_fp, UT_array *locus);

/**
 * Writes a single locus to the .bim file.
 *
 * @param bim_fp The .bim file to write to.
 * @param locus The locus to write.
 *
 * @return PIO_OK if the locus was successfully written,
 *         PIO_ERROR otherwise.
 */
pio_status_t write_locus(FILE *bim_fp, struct pio_locus_t *locus);

#ifdef __cplusplus
}
#endif

#endif /* End of __BIM_PARSE_H__ */
