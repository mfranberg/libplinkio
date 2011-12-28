#include <stdlib.h>
#include <string.h>

#include <plinkio.h>

int
pio_open(struct pio_file_t *plink_file, const char *plink_file_prefix)
{
    size_t path_length = strlen( plink_file_prefix ) + 4 + 1;
    char *path_buffer = (char *) malloc( sizeof( char ) * path_length );
    int error = 0;
    int num_samples;
    int num_loci;
    
    strncpy( path_buffer, plink_file_prefix, path_length );
    strncat( path_buffer, ".fam", path_length );
    if( fam_open( &plink_file->fam_file, path_buffer ) != PIO_OK )
    {
        error = 1;
    }

    strncpy( path_buffer, plink_file_prefix, path_length );
    strncat( path_buffer, ".bim", path_length );
    if( bim_open( &plink_file->bim_file, path_buffer ) != PIO_OK )
    {
        error = 1;
    }

    strncpy( path_buffer, plink_file_prefix, path_length );
    strncat( path_buffer, ".bed", path_length );
    num_samples = plink_file->fam_file.num_samples;
    num_loci = plink_file->bim_file.num_loci;
    if( bed_open( &plink_file->bed_file, path_buffer, num_samples, num_loci ) != PIO_OK )
    {
        error = 1;
    }

    free( path_buffer );
    if( error == 0 )
    {
        return PIO_OK;
    }
    else
    {
        return PIO_ERROR;
    }
}

struct pio_sample_t *
pio_get_sample(struct pio_file_t *plink_file, unsigned int pio_id)
{
    return fam_get_sample( &plink_file->fam_file, pio_id );
}

unsigned int
pio_num_samples(struct pio_file_t *plink_file)
{
    return fam_num_samples( &plink_file->fam_file );
}

struct pio_locus_t *
pio_get_locus(struct pio_file_t *plink_file, unsigned int pio_id)
{
    return bim_get_locus( &plink_file->bim_file, pio_id ); 
}

unsigned int
pio_num_loci(struct pio_file_t *plink_file)
{
    return bim_num_loci( &plink_file->bim_file );
}

unsigned int
pio_next_row(struct pio_file_t *plink_file, unsigned char *buffer)
{
    return bed_read_row( &plink_file->bed_file, buffer ); 
}

size_t
pio_row_size(struct pio_file_t *plink_file)
{
    return bed_row_size( &plink_file->bed_file );
}

enum SnpOrder
pio_row_order(struct pio_file_t *plink_file)
{
    return bed_snp_order( &plink_file->bed_file );
}

void
pio_close(struct pio_file_t *plink_file)
{
    bed_close( &plink_file->bed_file );
    bim_close( &plink_file->bim_file );
    fam_close( &plink_file->fam_file );
}
