#ifndef INCLUDED_PLINKIO_PRIVATE_FAM_H_
#define INCLUDED_PLINKIO_PRIVATE_FAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/fam.h>
#include <plinkio/status.h>

#include "private/sample.h"

/**
 * Link samplse to fam file.
 *
 * @param samples samples.
 * @param fam_file Fam file.
 * @param fam_path Fam file path.
 * @return PIO_OK if the locus could be written, PIO_ERROR otherwise.
 */
pio_status_t libplinkio_fam_link_samples_to_file_(libplinkio_samples_private_t samples, struct pio_fam_file_t* fam_file, const char* fam_path, _Bool is_tmp);


#ifdef __cplusplus
}
#endif
#endif // INCLUDED_PLINKIO_PRIVATE_FAM_H_
