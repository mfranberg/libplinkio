#include "plinkio/status.h"
#include "plinkio/fam.h"
#include "plinkio/bim.h"
#include "plinkio/bed.h"
#include "private/sample.h"
#include "private/locus.h"
#include "private/ped.h"
#include "private/ped_parse.h"

pio_status_t
libplinkio_ped_open_(libplinkio_samples_private_t *samples, libplinkio_loci_private_t *loci, struct pio_bed_file_t *bed_file, const char *path)
{
    FILE *ped_fp = NULL;
    *samples = libplinkio_init_samples_();

    ped_fp = fopen( path, "r" );
    if( ped_fp == NULL ) goto error;

    *samples = libplinkio_new_samples_();

    if (libplinkio_ped_parse_samples_(ped_fp, *samples, *loci, bed_file) != PIO_OK) goto error;

    fclose(ped_fp);

    return PIO_OK;

error:
    libplinkio_free_samples_(*samples);
    *samples = libplinkio_init_samples_();
    if (ped_fp != NULL) fclose(ped_fp);
    return PIO_ERROR;
}
