#ifndef INCLUDED_PLINKIO_PRIVATE_PED_H_
#define INCLUDED_PLINKIO_PRIVATE_PED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/status.h>
#include <plinkio/bed.h>
#include "private/sample.h"
#include "private/locus.h"

pio_status_t
libplinkio_ped_open_(libplinkio_samples_private_t *samples, libplinkio_loci_private_t *loci, struct pio_bed_file_t *bed_file, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* End of INCLUDED_PLINKIO_PRIVATE_PED_H_ */
