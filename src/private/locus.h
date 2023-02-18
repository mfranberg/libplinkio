#ifndef INCLUDED_PLINKIO_PRIVATE_LOCUS_H_
#define INCLUDED_PLINKIO_PRIVATE_LOCUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <plinkio/utarray.h>
#include <plinkio/bim.h>

#include "private/utility.h"

typedef struct {
    UT_array* ptr;
} libplinkio_loci_private_t;

/**
 * Locus destructor. Ensures that the allocated
 * strings are freed properly.
 *
 * @param element Pointer to a locus.
 */
static void
libplinkio_utarray_locus_dtor_(void *element)
{
    struct pio_locus_t *locus = (struct pio_locus_t *) element;

    if( locus->name != NULL )
    {
        free( locus->name );
    }
    if( locus->allele1 != NULL )
    {
        free( locus->allele1 );
    }
    if( locus->allele2 != NULL )
    {
        free( locus->allele2 );
    }
}

/**
 * Properties of the locus array for dtarray.
 */
static UT_icd LIBPLINKIO_LOCUS_ICD_ = { sizeof( struct pio_locus_t ), NULL, NULL, libplinkio_utarray_locus_dtor_ };

static FORCE_INLINE libplinkio_loci_private_t libplinkio_init_loci_(void) {
    libplinkio_loci_private_t loci = {0};
    return loci;
}

static FORCE_INLINE libplinkio_loci_private_t libplinkio_new_loci_(void) {
    libplinkio_loci_private_t loci = libplinkio_init_loci_();
    utarray_new( loci.ptr, &LIBPLINKIO_LOCUS_ICD_ );
    return loci;
}

static FORCE_INLINE size_t libplinkio_get_num_loci_(libplinkio_loci_private_t loci) {
    return utarray_len( loci.ptr );
}

static FORCE_INLINE struct pio_locus_t *
libplinkio_get_locus_(libplinkio_loci_private_t loci, size_t pio_id)
{
    return (struct pio_locus_t *) utarray_eltptr( loci.ptr, pio_id );  
}

static FORCE_INLINE struct pio_locus_t *
libplinkio_get_front_locus_(libplinkio_loci_private_t loci)
{
    return (struct pio_locus_t *) utarray_front( loci.ptr );  
}

static FORCE_INLINE struct pio_locus_t *
libplinkio_get_next_locus_(libplinkio_loci_private_t loci, struct pio_locus_t* prev_locus)
{
    return (struct pio_locus_t *) utarray_next(loci.ptr, prev_locus);
}

static FORCE_INLINE void libplinkio_add_locus_(libplinkio_loci_private_t loci, struct pio_locus_t* locus_ptr) {
    utarray_push_back( loci.ptr, locus_ptr );
}

static FORCE_INLINE void libplinkio_free_loci_(libplinkio_loci_private_t loci) {
    if (loci.ptr != NULL) utarray_free(loci.ptr);
}

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_PLINKIO_PRIVATE_LOCUS_H_
