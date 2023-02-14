#ifndef INCLUDED_PLINKIO_PRIVATE_BED_H_
#define INCLUDED_PLINKIO_PRIVATE_BED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/bed.h>
#include <plinkio/status.h>

pio_status_t
libplinkio_bed_transpose_fd_(const int original_fd, const int transposed_fd, size_t num_loci, size_t num_samples);

pio_status_t
libplinkio_bed_transpose_pio_bed_file_(struct pio_bed_file_t *bed_file, const char *transposed_path, size_t num_loci, size_t num_samples, _Bool is_tmp);

pio_status_t
libplinkio_bed_tmp_transposed_create_(struct pio_bed_file_t* bed_file, const char* path_prefix, size_t num_loci);

pio_status_t libplinkio_change_bed_read_only_ (struct pio_bed_file_t* bed_file);

#ifdef __cplusplus
}
#endif
#endif // INCLUDED_PLINKIO_PRIVATE_BED_H_
