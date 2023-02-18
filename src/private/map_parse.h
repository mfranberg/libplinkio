#ifndef INCLUDED_PLINKIO_PRIVATE_MAP_PARSE_H_
#define INCLUDED_PLINKIO_PRIVATE_MAP_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/status.h>

#include "private/locus.h"

pio_status_t
libplinkio_map_parse_loci_(FILE *map_fp, libplinkio_loci_private_t locus);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_PLINKIO_PRIVATE_MAP_PARSE_H_
