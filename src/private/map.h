#ifndef INCLUDED_PLINKIO_PRIVATE_MAP_H_
#define INCLUDED_PLINKIO_PRIVATE_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/status.h>

#include "private/locus.h"

pio_status_t
libplinkio_map_open_(libplinkio_loci_private_t *loci, const char *path);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_PLINKIO_PRIVATE_MAP_H_
