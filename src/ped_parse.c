/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include <plinkio/utarray.h>
#include <plinkio/bed.h>
#include <plinkio/bim.h>
#include <plinkio/status.h>

#include "private/utility.h"
#include "private/plink_txt_parse.h"
#include "private/ped_parse.h"

/**
 * Creates mock versions of IO functions to allow unit testing.
 */
#ifdef UNIT_TESTING
    extern size_t mock_fread(void *p, size_t size, size_t nmemb, FILE *stream);
    extern int mock_feof(FILE *stream);

    #define fread mock_fread
    #define feof mock_feof
#endif

/**
 * Buffer size for reading CSV file.
 */
#define LIBPLINKIO_PED_PARSE_BUFFER_SIZE_ 4096

struct ped_state_t
{
    /**
     * The next field to parse.
     */
    int field;

    /**
     * Determines whether there has been any error
     * during parsing.
     */
    int any_error;

    /**
     * Data of the sample that we are currently
     * parsing.
     */
    struct pio_sample_t cur_sample;

    /**
     * List of samples parsed so far.
     */
    libplinkio_samples_private_t samples;

    /**
     * List of alleles parsed so far.
     */
    libplinkio_loci_private_t loci;

    /**
     * Bed file.
     */
    struct pio_bed_file_t* bed_file;

    /**
     * Previous call of allele.
     */
    libplinkio_allele_call_private_t prev_call;

    /**
     * SNPs of current sample.
     */
    snp_t* snps;

    /**
     * Format of PED file.
     */
    libplinkio_ped_format_private_t format;
};

/**
 * Function that is called each time a new csv
 * field has been found. It is responsible for
 * determining which field is being parsed, and
 * updating the cur_sample object.
 *
 * @param field Csv field.
 * @param field_length Length of the field.
 * @param data A ped_state_t struct.
 */
static void
ped_new_field(char *field, size_t field_length, size_t field_num, void *data)
{
    UNUSED_PARAM(field_num);
    struct ped_state_t *state = (struct ped_state_t *) data;
    pio_status_t status = PIO_OK;
    char *buffer;

    size_t locus_length = 0;
    size_t idx = 0;
    size_t locus_idx = 0;
    size_t allele_idx = 0;

    if( state->field == -1 )
    {
        return;
    }

    /* Null terminated field. */
    buffer = (char * ) malloc( sizeof( char ) * ( field_length + 1 ) );
    strncpy( buffer, field, field_length );
    buffer[ field_length ] = '\0';

    switch( state->field )
    {
        case 0:
            state->cur_sample.fid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 1:
            state->cur_sample.iid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 2:
            state->cur_sample.father_iid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 3:
            state->cur_sample.mother_iid = libplinkio_parse_str_( buffer, field_length, &status );
            break;
        case 4:
            state->cur_sample.sex = libplinkio_parse_sex_( buffer, field_length, &status );
            break;
        case 5:
            libplinkio_parse_phenotype_( buffer, field_length, &state->cur_sample, &status );
            break;
        default:
            locus_length = libplinkio_get_num_loci_(state->loci);
            idx = state->field - 6;
            locus_idx = 0;
            allele_idx = 0;

            switch (state->format) {
                case LIBPLINKIO_PED_SIMPLE_:
                    if (idx >= locus_length*2) {
                        status = PIO_ERROR;
                        goto error;
                        break;
                    }
                    locus_idx = idx >> 1;
                    allele_idx = idx & 1;
                    libplinkio_parse_allele_(
                        buffer,
                        field_length,
                        locus_idx,
                        allele_idx,
                        state->loci,
                        &state->prev_call,
                        state->snps,
                        &status
                    );
                    break;
                case LIBPLINKIO_PED_COMPOUND_:
                    if (idx >= locus_length || field_length != 2) {
                        status = PIO_ERROR;
                        goto error;
                        break;
                    }
                    libplinkio_parse_allele_(
                        buffer,
                        1,
                        idx,
                        0,
                        state->loci,
                        &state->prev_call,
                        state->snps,
                        &status
                    );
                    libplinkio_parse_allele_(
                        buffer + 1,
                        1,
                        idx,
                        1,
                        state->loci,
                        &state->prev_call,
                        state->snps,
                        &status
                    );
                    break;
            }
            break;
    }

    free( buffer );

    if( status != PIO_OK ) goto error;
    state->field++;
    return;

error:
    state->any_error = 1;
    state->field = -1;
    return;
}

/**
 * Function that is called each time a new row
 * has been found.
 *
 * @param number The row number.
 * @param data A ped_state_t struct.
 */
static void
ped_new_row(size_t number, void *data)
{
    UNUSED_PARAM(number);
    struct ped_state_t *state = (struct ped_state_t *) data;
    size_t expected_num_cols = 0;
    if (state->format == LIBPLINKIO_PED_SIMPLE_) {
        expected_num_cols = libplinkio_get_num_loci_(state->loci)*2 + 6;
    } else {
        // state->format == LIBPLINKIO_PED_COMPOUND_
        expected_num_cols = libplinkio_get_num_loci_(state->loci) + 6;
    }

    if( (state->field - expected_num_cols) == 0 )
    {
        state->cur_sample.pio_id = libplinkio_get_num_samples_( state->samples );
        libplinkio_add_sample_( state->samples, &state->cur_sample );
        state->cur_sample = (struct pio_sample_t){ 0 };
        bed_write_row(state->bed_file, state->snps);
        memset(state->snps, 0, sizeof(snp_t)*libplinkio_get_num_loci_(state->loci));
    } else {
        state->any_error = 1;
    }
    state->field = 0;
}

pio_status_t
libplinkio_ped_parse_samples_(FILE* ped_fp, libplinkio_samples_private_t samples, libplinkio_loci_private_t loci, struct pio_bed_file_t* bed_file)
{
    char read_buffer[ LIBPLINKIO_PED_PARSE_BUFFER_SIZE_ ];
    struct ped_state_t state = { 0 };
    libplinkio_txt_parser_private_t parser = { 0 };

    int ped_num_cols = libplinkio_count_txt_column_(ped_fp);
    if ( (ped_num_cols < 0) ) goto error;

    state.samples = samples;
    state.loci = loci;
    state.bed_file = bed_file;

    size_t locus_length = libplinkio_get_num_loci_(state.loci);

    if ((locus_length*2) - (ped_num_cols - 6) == 0) {
        state.format = LIBPLINKIO_PED_SIMPLE_;
    } else if (locus_length - (ped_num_cols - 6) == 0) {
        state.format = LIBPLINKIO_PED_COMPOUND_;
    } else {
        goto error;
    }

    state.snps = (snp_t*)calloc(locus_length, sizeof(snp_t));
    if (state.snps == NULL) goto error;

    libplinkio_txt_parser_init_( &parser );
    do {
        size_t bytes_read = fread( read_buffer, sizeof( char ), LIBPLINKIO_PED_PARSE_BUFFER_SIZE_ - 1, ped_fp );
        if (ferror( ped_fp )) goto error;
        read_buffer[bytes_read] = '\0';
        libplinkio_txt_parse_( &parser, read_buffer, bytes_read, &ped_new_field, &ped_new_row, (void *) &state );
    } while( !feof( ped_fp ) );

    libplinkio_txt_parse_fini_( &parser, &ped_new_field, &ped_new_row, (void *) &state );
    libplinkio_txt_parser_free_( &parser );

    free(state.snps);
    state.snps = NULL;

    if ( state.any_error != 0 ) goto error;
    return PIO_OK;

error:
    free(state.snps);
    state.snps = NULL;
    return PIO_ERROR;
}


