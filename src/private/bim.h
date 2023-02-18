#ifndef INCLUDED_PLINKIO_PRIVATE_BIM_H_
#define INCLUDED_PLINKIO_PRIVATE_BIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/bim.h>
#include <plinkio/status.h>

#include "private/locus.h"

/**
 * Link loci to bim file.
 *
 * @param loci Loci.
 * @param bim_file Bim file.
 * @param bim_path Bim file path.
 * @return PIO_OK if the locus could be written, PIO_ERROR otherwise.
 */
pio_status_t libplinkio_bim_link_loci_to_file_(libplinkio_loci_private_t loci, struct pio_bim_file_t* bim_file, const char* bim_path, _Bool is_tmp);

#ifdef __cplusplus
}
#endif
#endif // INCLUDED_PLINKIO_PRIVATE_BIM_H_
