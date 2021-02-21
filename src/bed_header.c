/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <plinkio/bed_header.h>

/**
 * Magic constants for v 1.00 format.
 */
#define BED_V100_MAGIC1 0x6c
#define BED_V100_MAGIC2 0x1b

/**
 * Mask for SNP order. 
 */
#define BED_SNP_ORDER_BIT 0x01

/**
 * Returns the SNP order encoded in
 * a byte.
 *
 * @param order SNP order encoded in a byte.
 *
 * @return ONE_LOCUS_PER_ROW if highest bit in order is set,
 *         ONE_SAMPLE_PER_ROW otherwise.
 */
int
get_snp_order(unsigned char order)
{
    if( order == BED_SNP_ORDER_BIT )
    {
        return BED_ONE_LOCUS_PER_ROW;
    }
    else if( order == 0 )
    {
        return BED_ONE_SAMPLE_PER_ROW;
    }
    else
    {
        return BED_UNKNOWN_ORDER;
    }
}

/**
 * Returns the file offset to the data for the 
 * given version.
 *
 * @param version Version of the bed file.
 *
 * @return File offset to the data section.
 */
int
get_data_offset(enum BedVersion version)
{
    if( version == PIO_VERSION_100 )
    {
        return 3;
    }
    else if( version == PIO_VERSION_099 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * Picks out the version and snp order from the file header.
 *
 * @param header First 3 bytes of the bed file.
 * @param version The version will be stored here.
 * @param snp_order The snp order will be stored here.
 */
void
get_version_and_order(const unsigned char *header, enum BedVersion *version, enum SnpOrder *snp_order)
{
    if( header[ 0 ] == BED_V100_MAGIC1 && header[ 1 ] == BED_V100_MAGIC2 )
    {
        /* Version 1.00 */
        *version = PIO_VERSION_100;
        *snp_order = get_snp_order( header[ 2 ] );

    }
    else if( ( header[ 0 ] & ~BED_SNP_ORDER_BIT ) == 0 )
    {
        /* Version 0.99 (hopefully) */
        *version = PIO_VERSION_099;
        *snp_order = get_snp_order( header[ 0 ] );
    }
    else
    {
        /* Version < 0.99 */
        *version = PIO_VERSION_PRE_099;
        *snp_order = BED_ONE_SAMPLE_PER_ROW;
    }
}

unsigned char
snp_order_as_byte(enum SnpOrder snp_order)
{
    if( snp_order == BED_ONE_LOCUS_PER_ROW )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * Sets the snp order of the header in the position according
 * to the version.
 *
 * @param header The header to set the snp order in.
 * @param version The version of the bed file.
 * @param snp_order The order to set.
 */
void
set_order(unsigned char *header, enum BedVersion version, enum SnpOrder snp_order)
{
    unsigned char snp_order_as_byte = 1;
    if( snp_order == BED_ONE_SAMPLE_PER_ROW )
    {
        snp_order_as_byte = 0;
    }

    if( version == PIO_VERSION_100 )
    {
        header[ 2 ] = snp_order_as_byte;
    }
    else if( version == PIO_VERSION_099 )
    {
        header[ 0 ] = snp_order_as_byte;
    }
}

struct bed_header_t
bed_header_init(size_t num_loci, size_t num_samples)
{
    struct bed_header_t header = { 0 };
    header.num_loci = num_loci;
    header.num_samples = num_samples;
    header.version = PIO_VERSION_100;
    header.snp_order = BED_ONE_LOCUS_PER_ROW;

    return header;
}

struct bed_header_t
bed_header_init2(size_t num_loci, size_t num_samples, const unsigned char *header_bytes)
{
    struct bed_header_t header = bed_header_init( num_loci, num_samples );
    bed_header_from_bytes( &header, header_bytes );

    return header;
}

void
bed_header_from_bytes(struct bed_header_t *header, const unsigned char *header_bytes)
{
    get_version_and_order( header_bytes, &header->version, &header->snp_order );
}

void
bed_header_to_bytes(struct bed_header_t *header, unsigned char *header_bytes, size_t *length)
{
    if( header->version == PIO_VERSION_100 )
    {
        header_bytes[ 0 ] = BED_V100_MAGIC1;
        header_bytes[ 1 ] = BED_V100_MAGIC2;
        header_bytes[ 2 ] = snp_order_as_byte( header->snp_order );
    }
    else if( header->version == PIO_VERSION_099 )
    {
        header_bytes[ 0 ] = snp_order_as_byte( header->snp_order );
    }
    *length = get_data_offset( header->version );
}

size_t
bed_header_num_rows(struct bed_header_t *header)
{
    if( header->snp_order == BED_ONE_LOCUS_PER_ROW )
    {
        return header->num_loci;
    }
    else
    {
        return header->num_samples;
    }
}

size_t
bed_header_num_cols(struct bed_header_t *header)
{
    if( header->snp_order == BED_ONE_LOCUS_PER_ROW )
    {
        return header->num_samples;
    }
    else
    {
        return header->num_loci;
    }
}

size_t
bed_header_data_offset(struct bed_header_t *header)
{
    return get_data_offset( header->version );
}

size_t
bed_header_row_size(struct bed_header_t *header)
{
    return ( bed_header_num_cols( header ) + 3 ) / 4;
}

size_t
bed_header_data_size(struct bed_header_t *header)
{
    size_t bytes_per_row = bed_header_row_size( header );
    return bed_header_data_offset( header ) + bed_header_num_rows( header ) * bytes_per_row;
}

size_t
bed_header_file_size(struct bed_header_t *header)
{
    return bed_header_data_offset( header ) +
        bed_header_data_size( header );
}

enum SnpOrder
bed_header_snp_order(struct bed_header_t *header)
{
    return header->snp_order;
}

void
bed_header_transpose(struct bed_header_t *header)
{
    if( header->snp_order == BED_ONE_LOCUS_PER_ROW )
    {
        header->snp_order = BED_ONE_SAMPLE_PER_ROW;
    }
    else
    {
        header->snp_order = BED_ONE_LOCUS_PER_ROW;
    }
}
