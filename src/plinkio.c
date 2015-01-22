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
#include <plinkio/file.h>

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
    char *fam_path = concatenate( plink_file_prefix, ".fam" );
    char *bim_path = concatenate( plink_file_prefix, ".bim" );
    char *bed_path = concatenate( plink_file_prefix, ".bed" );

    pio_status_t status = pio_open_ex( plink_file, fam_path, bim_path, bed_path );

    free( fam_path );
    free( bim_path );
    free( bed_path );

    return status;
}


pio_status_t pio_open_ex(struct pio_file_t *plink_file, const char *fam_path, const char *bim_path, const char *bed_path)
{
    int error = 0;
    int num_samples = 0;
    int num_loci = 0;

    if( fam_open( &plink_file->fam_file, fam_path ) == PIO_OK )
    {
        num_samples = fam_num_samples( &plink_file->fam_file );
    }
    else
    {
        error = P_FAM_IO_ERROR;
    }

    if( bim_open( &plink_file->bim_file, bim_path ) == PIO_OK )
    {
        num_loci = bim_num_loci( &plink_file->bim_file );
    }
    else
    {
        error = P_BIM_IO_ERROR;
    }

    if( bed_open( &plink_file->bed_file, bed_path, num_loci, num_samples ) != PIO_OK )
    {
        error = P_BED_IO_ERROR;
    }

    if( error == 0 )
    {
        return PIO_OK;
    }
    else
    {
        fam_close( &plink_file->fam_file );
        bim_close( &plink_file->bim_file );
        bed_close( &plink_file->bed_file );

        return error;
    }
}

pio_status_t
pio_create(struct pio_file_t *plink_file, const char *plink_file_prefix, struct pio_sample_t *samples, size_t num_samples)
{
    char *fam_path = concatenate( plink_file_prefix, ".fam" );
    char *bim_path = concatenate( plink_file_prefix, ".bim" );
    char *bed_path = concatenate( plink_file_prefix, ".bed" );
    
    if( fam_create( &plink_file->fam_file, fam_path, samples, num_samples ) != PIO_OK )
    {
        return P_FAM_IO_ERROR;
    }
    if( bim_create( &plink_file->bim_file, bim_path ) != PIO_OK )
    {
        return P_BIM_IO_ERROR;
    }
    if( bed_create( &plink_file->bed_file, bed_path, num_samples ) != PIO_OK )
    {
        return P_BED_IO_ERROR;
    }

    return PIO_OK;
}

pio_status_t
pio_write_row(struct pio_file_t *plink_file, struct pio_locus_t *locus, snp_t *buffer)
{
    pio_status_t status_bim = bim_write( &plink_file->bim_file, locus );
    pio_status_t status_bed = bed_write_row( &plink_file->bed_file, buffer );

    if( status_bim != PIO_OK )
    {
        return P_BIM_IO_ERROR;
    }
    else if( status_bed != PIO_OK )
    {
        return P_BED_IO_ERROR;
    }
    else
    {
        return PIO_OK;
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
