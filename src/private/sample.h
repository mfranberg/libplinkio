#ifndef __PLINKIO_PRIVATE_SAMPLE_H__
#define __PLINKIO_PRIVATE_SAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/utarray.h>
#include <plinkio/fam.h>

#include "private/utility.h"

typedef struct {
    UT_array* ptr;
} libplinkio_samples_private_t;

/**
 * Sample destructor. Ensures that the allocated
 * strings are freed properly.
 *
 * @param element Pointer to a sample.
 */
static void
libplinkio_utarray_sample_dtor_(void *element)
{
    struct pio_sample_t *sample = (struct pio_sample_t *) element;

    if( sample->fid != NULL )
    {
        free( sample->fid );
    }
    if( sample->iid != NULL )
    {
        free( sample->iid );
    }
    if( sample->father_iid != NULL )
    {
        free( sample->father_iid );
    }
    if( sample->mother_iid != NULL )
    {
        free( sample->mother_iid );
    }
}

/**
 * Properties of the sample array for dtarray.
 */
static UT_icd LIBPLINKIO_SAMPLE_ICD_ = {
    sizeof( struct pio_sample_t ),
    NULL,
    NULL,
    libplinkio_utarray_sample_dtor_
};

static FORCE_INLINE libplinkio_samples_private_t libplinkio_init_samples_(void) {
    libplinkio_samples_private_t samples = {0};
    return samples;
}

static FORCE_INLINE libplinkio_samples_private_t libplinkio_new_samples_(void) {
    libplinkio_samples_private_t samples = {0};
    utarray_new( samples.ptr, &LIBPLINKIO_SAMPLE_ICD_ );
    return samples;
}

static FORCE_INLINE size_t libplinkio_get_num_samples_(libplinkio_samples_private_t samples) {
    return utarray_len( samples.ptr );
}

static FORCE_INLINE struct pio_sample_t *
libplinkio_get_sample_(libplinkio_samples_private_t samples, size_t pio_id)
{
    return (struct pio_sample_t *) utarray_eltptr( samples.ptr, pio_id );  
}

static FORCE_INLINE struct pio_sample_t *
libplinkio_get_front_sample_(libplinkio_samples_private_t samples)
{
    return (struct pio_sample_t *) utarray_front( samples.ptr );  
}

static FORCE_INLINE struct pio_sample_t *
libplinkio_get_next_sample_(libplinkio_samples_private_t samples, struct pio_sample_t* prev_sample)
{
    return (struct pio_sample_t *) utarray_next(samples.ptr, prev_sample);
}

static FORCE_INLINE void libplinkio_add_sample_(libplinkio_samples_private_t samples, struct pio_sample_t* sample_ptr) {
    utarray_push_back( samples.ptr, sample_ptr );
}

static FORCE_INLINE void libplinkio_free_samples_(libplinkio_samples_private_t samples) {
    if (samples.ptr != NULL) utarray_free(samples.ptr);
}

#ifdef __cplusplus
}
#endif

#endif // __PLINKIO_PRIVATE_SAMPLE_H__
