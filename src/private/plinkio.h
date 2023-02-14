#ifndef INCLUDED_PLINKIO_PRIVATE_PLINKIO_H_
#define INCLUDED_PLINKIO_PRIVATE_PLINKIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/status.h>

pio_status_t
libplinkio_open_txt_ex_(
    struct pio_file_t *plink_file,
    const char *ped_path,
    const char *map_path,
    const char *fam_path,
    const char *bim_path,
    const char *bed_path,
    _Bool is_tmp
);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_PLINKIO_PRIVATE_PLINKIO_H_
