#ifndef INCLUDED_PLINKIO_PRIVATE_PACKED_SNP_H_
#define INCLUDED_PLINKIO_PRIVATE_PACKED_SNP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/status.h>
#include <plinkio/bed.h>

#include "private/locus.h"

pio_status_t libplinkio_flip_alleles_(libplinkio_loci_private_t loci, struct pio_bed_file_t* bed_file, size_t num_samples);

#ifdef __cplusplus
}
#endif

#endif /* End of INCLUDED_PLINKIO_PRIVATE_PACKED_SNP_H_ */
