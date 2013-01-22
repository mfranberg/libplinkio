/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __FAM_PARSE_H__
#define __FAM_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <plinkio/fam.h>
#include <plinkio/status.h>

/**
 * Parses the samples and points the given sample array to a
 * the memory that contains them, and writes back the number
 * of samples.
 *
 * @param fam_fp Fam file.
 * @param sample Parsed samples will be stored here.
 *
 * @return PIO_OK if the samples could be parsed, PIO_ERROR otherwise.
 */
pio_status_t parse_samples(FILE *fam_fp, UT_array *sample);

#ifdef __cplusplus
}
#endif

#endif /* End of __FAM_PARSE_H__ */
