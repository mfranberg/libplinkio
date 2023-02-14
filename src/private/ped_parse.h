#ifndef INCLUDED_PLINKIO_PRIVATE_PED_PARSE_H_
#define INCLUDED_PLINKIO_PRIVATE_PED_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/status.h>

#include "private/sample.h"
#include "private/locus.h"

typedef enum {
    LIBPLINKIO_PED_SIMPLE_,
    LIBPLINKIO_PED_COMPOUND_
} libplinkio_ped_format_private_t;

pio_status_t
libplinkio_ped_parse_samples_(FILE* ped_fp, libplinkio_samples_private_t samples, libplinkio_loci_private_t loci, struct pio_bed_file_t* bed_file);

#ifdef __cplusplus
}
#endif

#endif /* End of INCLUDED_PLINKIO_PRIVATE_PED_PARSE_H_ */
