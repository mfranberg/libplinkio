/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdlib.h>
#include <string.h>

#include <plinkio/plinkio.h>

/**
 * Concatenates the given strings and returns the concatenated
 * string a + b.
 *
 * Note: User is responsible for calling free on the returned
 * string.
 *
 * @param a First string.
 * @param b Second string.
 *
 * @return pointer to the concatenated string.
 */
char *
concatenate(const char *a, const char *b)
{
    size_t total_length = strlen( a ) + strlen( b ) + 1;
    char *buffer = (char *) malloc( total_length );
    strncpy( buffer, a, total_length );
    strncat( buffer, b, total_length );

    return buffer;
}

pio_status_t
pio_open(struct pio_file_t *plink_file, const char *plink_file_prefix)
{
    int error = 0;
    int num_samples = 0;
    int num_loci = 0;
    
    char *fam_path = concatenate( plink_file_prefix, ".fam" );
    if( fam_open( &plink_file->fam_file, fam_path ) == PIO_OK )
    {
        num_samples = fam_num_samples( &plink_file->fam_file );
    }
    else
    {
        error = 1;
    }

    char *bim_path = concatenate( plink_file_prefix, ".bim" );
    if( bim_open( &plink_file->bim_file, bim_path ) == PIO_OK )
    {
        num_loci = bim_num_loci( &plink_file->bim_file );
    }
    else
    {
        error = 1;
    }

    char *bed_path = concatenate( plink_file_prefix, ".bed" );
    if( bed_open( &plink_file->bed_file, bed_path, num_loci, num_samples ) != PIO_OK )
    {
        error = 1;
    }

    free( fam_path );
    free( bim_path );
    free( bed_path );
    if( error == 0 )
    {
        return PIO_OK;
    }
    else
    {
        fam_close( &plink_file->fam_file );
        bim_close( &plink_file->bim_file );
        bed_close( &plink_file->bed_file );

        return PIO_ERROR;
    }
}

struct pio_sample_t *
pio_get_sample(struct pio_file_t *plink_file, size_t pio_id)
{
    return fam_get_sample( &plink_file->fam_file, pio_id );
}

size_t
pio_num_samples(struct pio_file_t *plink_file)
{
    return fam_num_samples( &plink_file->fam_file );
}

struct pio_locus_t *
pio_get_locus(struct pio_file_t *plink_file, size_t pio_id)
{
    return bim_get_locus( &plink_file->bim_file, pio_id ); 
}

size_t
pio_num_loci(struct pio_file_t *plink_file)
{
    return bim_num_loci( &plink_file->bim_file );
}

pio_status_t
pio_next_row(struct pio_file_t *plink_file, snp_t *buffer)
{
    return bed_read_row( &plink_file->bed_file, buffer ); 
}

void
pio_reset_row(struct pio_file_t *plink_file)
{
    bed_reset_row( &plink_file->bed_file );
}

size_t
pio_row_size(struct pio_file_t *plink_file)
{
    return bed_row_size( &plink_file->bed_file );
}

int
pio_one_locus_per_row(struct pio_file_t *plink_file)
{
    return bed_snp_order( &plink_file->bed_file) == BED_ONE_LOCUS_PER_ROW;
}

void
pio_close(struct pio_file_t *plink_file)
{
    bed_close( &plink_file->bed_file );
    bim_close( &plink_file->bim_file );
    fam_close( &plink_file->fam_file );
}

pio_status_t
pio_transpose(const char *plink_file_prefix, const char *transposed_file_prefix)
{
    struct pio_file_t plink_file;
    if( pio_open( &plink_file, plink_file_prefix ) != PIO_OK )
    {
        return PIO_ERROR;
    }

    char *bed_path = concatenate( plink_file_prefix, ".bed" );
    char *transposed_bed_path = concatenate( transposed_file_prefix, ".bed" );

    pio_status_t status = bed_transpose( bed_path, transposed_bed_path, pio_num_loci( &plink_file ), pio_num_samples( &plink_file ) );
    if( status == PIO_OK )
    {
        char *fam_path = concatenate( plink_file_prefix, ".fam" );
        char *transposed_fam_path = concatenate( transposed_file_prefix, ".fam" );
        file_copy( fam_path, transposed_fam_path );
        free( fam_path );
        free( transposed_fam_path );
        
        char *bim_path = concatenate( plink_file_prefix, ".bim" );
        char *transposed_bim_path = concatenate( transposed_file_prefix, ".bim" );
        file_copy( bim_path, transposed_bim_path );
        free( bim_path );
        free( transposed_bim_path );
    }

    pio_close( &plink_file );

    free( bed_path );
    free( transposed_bed_path );

    return status;
}
