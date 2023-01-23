#ifndef __FAM_PED_PARSE_H__
#define __FAM_PED_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <plinkio/plinkio.h>
#include <plinkio/status.h>

typedef enum {
    PIO_SIMPLE_PARSER,
    PIO_COMPOUND_PED_PARSER
} pio_txt_parser_mode_t;

typedef enum {
    PIO_TOKEN_INIT,
    PIO_TOKEN_GRAPH,
    PIO_TOKEN_DELIM,
    PIO_TOKEN_EOL,
    PIO_TOKEN_EOF
} pio_txt_parser_state_t;

typedef struct {
    pio_txt_parser_mode_t mode;
    pio_txt_parser_state_t prev_state;
    size_t field_buffer_size;
    char* field_buffer;
    size_t field_length;
    size_t field_num;
    size_t row_num;
} pio_txt_parser_t;

char*
pio_parse_str(const char *field, size_t length, pio_status_t *status);

unsigned char
pio_parse_chr(const char *field, size_t length, pio_status_t *status);

float
pio_parse_genetic_position(const char *field, size_t length, pio_status_t *status);

long long
pio_parse_bp_position(const char *field, size_t length, pio_status_t *status);

enum sex_t
pio_parse_sex(const char *field, size_t length, pio_status_t *status);

void
pio_parse_phenotype(const char *field, size_t length, struct pio_sample_t *sample, pio_status_t *status);

pio_status_t
pio_txt_parser_init(
    pio_txt_parser_t* parser,
    pio_txt_parser_mode_t parser_mode
);

pio_status_t
pio_txt_parse(
    pio_txt_parser_t* parser,
    char* buffer,
    size_t length,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
);

pio_status_t
pio_txt_parse_fini(
    pio_txt_parser_t* parser,
    void (*new_field)(char*, size_t, size_t, void*),
    void (*new_row)(size_t, void*),
    void* data
);

void
pio_txt_parser_free(
    pio_txt_parser_t* parser
);

#ifdef __cplusplus
}
#endif

#endif /* End of __FAM_PED_PARSE_H__ */
