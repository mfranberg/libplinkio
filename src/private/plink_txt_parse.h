#ifndef INCLUDED_PLINKIO_PRIVATE_PLINK_TXT_PARSE_H_
#define INCLUDED_PLINKIO_PRIVATE_PLINK_TXT_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <plinkio/plinkio.h>
#include <plinkio/status.h>

#include "private/locus.h"

typedef enum {
    LIBPLINKIO_CHAR_SET_INIT_,
    LIBPLINKIO_CHAR_SET_GRAPH_,
    LIBPLINKIO_CHAR_SET_DELIM_,
    LIBPLINKIO_CHAR_SET_EOL_,
    LIBPLINKIO_CHAR_SET_EOF_
} libplinkio_txt_parser_state_private_t;

typedef struct {
    libplinkio_txt_parser_state_private_t prev_state;
    size_t field_buffer_size;
    char* field_buffer;
    size_t field_length;
    size_t field_num;
    size_t row_num;
} libplinkio_txt_parser_private_t;

typedef enum {
    LIBPLINKIO_ALLELE_CALL_NO_ = 0, // Previous call is nothing.
    LIBPLINKIO_ALLELE_CALL_1_ = 1, // Previous call is the first allele.
    LIBPLINKIO_ALLELE_CALL_2_ = 2, // Previous call is the second allele.
    LIBPLINKIO_ALLELE_CALL_ERROR_ = -1 // Error
} libplinkio_allele_call_private_t;

char*
libplinkio_parse_str_(const char *field, size_t length, pio_status_t *status);

unsigned char
libplinkio_parse_chr_(const char *field, size_t length, pio_status_t *status);

float
libplinkio_parse_genetic_position_(const char *field, size_t length, pio_status_t *status);

long long
libplinkio_parse_bp_position_(const char *field, size_t length, pio_status_t *status);

enum sex_t
libplinkio_parse_sex_(const char *field, size_t length, pio_status_t *status);

void
libplinkio_parse_phenotype_(const char *field, size_t length, struct pio_sample_t *sample, pio_status_t *status);

void
libplinkio_parse_allele_(const char *field, size_t length, size_t locus_idx, size_t allele_idx, libplinkio_loci_private_t loci, libplinkio_allele_call_private_t* prev_call, snp_t* snps, pio_status_t* status);

pio_status_t
libplinkio_txt_parser_init_(
    libplinkio_txt_parser_private_t* parser
);

pio_status_t
libplinkio_txt_parse_(
    libplinkio_txt_parser_private_t* parser,
    char* buffer,
    size_t length,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
);

pio_status_t
libplinkio_txt_parse_fini_(
    libplinkio_txt_parser_private_t* parser,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
);

void
libplinkio_txt_parser_free_(
    libplinkio_txt_parser_private_t* parser
);

int libplinkio_count_txt_column_(FILE* fp);

#ifdef __cplusplus
}
#endif

#endif /* End of INCLUDED_PLINKIO_PRIVATE_PLINK_TXT_PARSE_H_ */
