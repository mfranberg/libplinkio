#include "private/packed_snp.h"
#include "private/utility.h"
#include "private/locus.h"

#include <limits.h>
#include <stdlib.h>

static FORCE_INLINE uint8_t flip_allele8(uint8_t x) {
    uint8_t y = (~(x ^ ((x & 0xAA) >> 1))) & 0x55;
    y |= y << 1;
    return (~x & y) | (x & ~y);
}

static FORCE_INLINE void flip_alleles8(uint8_t* x, size_t length) {
    for (size_t i = 0; i < length; i++) {
        x[i] = flip_allele8(x[i]);
    }
}

static FORCE_INLINE uint8_t cnt_first_allele8(uint8_t x) {
    uint8_t y = (~(x ^ ((x & 0xaa) >> 1))) & 0x55;
    y |= y << 1;
    return libplinkio_popcnt8_((~x & y) | (x & ~y & 0xaa));
}

static FORCE_INLINE uint8_t cnt_first_alleles8(const uint8_t* x, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_first_allele8(x[i]);
    }
    return sum;
}

static FORCE_INLINE uint8_t cnt_second_allele8(uint8_t x) {
    uint8_t y = (~(x ^ ((x & 0xaa) >> 1))) & 0x55;
    y |= y << 1;
    return libplinkio_popcnt8_((x & y) | (x & ~y & 0xaa));
}

static FORCE_INLINE uint8_t cnt_second_alleles8(const uint8_t* x, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_second_allele8(x[i]);
    }
    return sum;
}

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
static FORCE_INLINE uint16_t flip_allele16(uint16_t x) {
    uint16_t y = (~(x ^ ((x & 0xaaaa) >> 1))) & 0x5555;
    y |= y << 1;
    return (~x & y) | (x & ~y);
}

static FORCE_INLINE void flip_alleles16(uint16_t* x, size_t length) {
    for (size_t i = 0; i < length; i++) {
        x[i] = flip_allele16(x[i]);
    }
}

static FORCE_INLINE uint16_t cnt_first_allele16(uint16_t x) {
    uint16_t y = (~(x ^ ((x & 0xaaaa) >> 1))) & 0x5555;
    y |= y << 1;
    return libplinkio_popcnt16_((~x & y) | (x & ~y & 0xaaaa));
}

static FORCE_INLINE uint16_t cnt_first_alleles16(const uint16_t* x, size_t length) {
    uint16_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_first_allele16(x[i]);
    }
    return sum;
}

static FORCE_INLINE uint16_t cnt_second_allele16(uint16_t x) {
    uint16_t y = (~(x ^ ((x & 0xaaaa) >> 1))) & 0x5555;
    y |= y << 1;
    return libplinkio_popcnt16_((x & y) | (x & ~y & 0xaaaa));
}

static FORCE_INLINE uint16_t cnt_second_alleles16(const uint16_t* x, size_t length) {
    uint16_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_second_allele16(x[i]);
    }
    return sum;
}
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
static FORCE_INLINE uint32_t flip_allele32(uint32_t x) {
    uint32_t y = (~(x ^ ((x & 0xaaaaaaaa) >> 1))) & 0x55555555;
    y |= y << 1;
    return (~x & y) | (x & ~y);
}

static FORCE_INLINE void flip_alleles32(uint32_t* x, size_t length) {
    for (size_t i = 0; i < length; i++) {
        x[i] = flip_allele32(x[i]);
    }
}

static FORCE_INLINE uint32_t cnt_first_allele32(uint32_t x) {
    uint32_t y = (~(x ^ ((x & 0xaaaaaaaa) >> 1))) & 0x55555555;
    y |= y << 1;
    return libplinkio_popcnt32_((~x & y) | (x & ~y & 0xaaaaaaaa));
}

static FORCE_INLINE uint32_t cnt_first_alleles32(const uint32_t* x, size_t length) {
    uint32_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_first_allele32(x[i]);
    }
    return sum;
}

static FORCE_INLINE uint32_t cnt_second_allele32(uint32_t x) {
    uint32_t y = (~(x ^ ((x & 0xaaaaaaaa) >> 1))) & 0x55555555;
    y |= y << 1;
    return libplinkio_popcnt32_((x & y) | (x & ~y & 0xaaaaaaaa));
}

static FORCE_INLINE uint32_t cnt_second_alleles32(const uint32_t* x, size_t length) {
    uint32_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_second_allele32(x[i]);
    }
    return sum;
}
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 64 == 0)
static FORCE_INLINE uint64_t flip_allele64(uint64_t x) {
    uint64_t y = (~(x ^ ((x & 0xaaaaaaaaaaaaaaaa) >> 1))) & 0x5555555555555555;
    y |= y << 1;
    return (~x & y) | (x & ~y);
}

static FORCE_INLINE void flip_alleles64(uint64_t* x, size_t length) {
    for (size_t i = 0; i < length; i++) {
        x[i] = flip_allele64(x[i]);
    }
}

static FORCE_INLINE uint64_t cnt_first_allele64(uint64_t x) {
    uint64_t y = (~(x ^ ((x & 0xaaaaaaaaaaaaaaaa) >> 1))) & 0x5555555555555555;
    y |= y << 1;
    return libplinkio_popcnt64_((~x & y) | (x & ~y & 0xaaaaaaaaaaaaaaaa));
}

static FORCE_INLINE uint64_t cnt_first_alleles64(const uint64_t* x, size_t length) {
    uint64_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_first_allele64(x[i]);
    }
    return sum;
}

static FORCE_INLINE uint64_t cnt_second_allele64(uint64_t x) {
    uint64_t y = (~(x ^ ((x & 0xaaaaaaaaaaaaaaaa) >> 1))) & 0x5555555555555555;
    y |= y << 1;
    return libplinkio_popcnt64_((x & y) | (x & ~y & 0xaaaaaaaaaaaaaaaa));
}

static FORCE_INLINE uint64_t cnt_second_alleles64(const uint64_t* x, size_t length) {
    uint64_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += cnt_second_allele64(x[i]);
    }
    return sum;
}
#endif

static FORCE_INLINE size_t get_alleles_length_from_num_cols(size_t num_cols) {
    size_t length = num_cols >> 2;
    if (num_cols - (length << 2) > 0) length++;
    return length;
}

static size_t cnt_first_alleles(const uint8_t* x, size_t num_cols) {
    uintptr_t offset;
    size_t length = get_alleles_length_from_num_cols(num_cols);
    size_t sum = 0;

    if (length == 0) return 0;

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
    offset = (uintptr_t)x & 0b1;
    sum += cnt_first_alleles8(x, offset);
    length -= offset;
    x += offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
    offset = (uintptr_t)x & 0b10;
    if (length >= offset) {
        sum += cnt_first_alleles16((uint16_t*)x, offset >> 1);
        length -= offset;
        x += offset;
    }
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 64 == 0)
    offset = (uintptr_t)x & 0b100;
    if (length >= offset) {
        sum += cnt_first_alleles32((uint32_t*)x, offset >> 2);
        length -= offset;
        x += offset;
    }

    sum += cnt_first_alleles64((uint64_t*)x, length >> 3);
    offset = length & ~(uintptr_t)0b111;
    x += offset;
    length -= offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
    sum += cnt_first_alleles32((uint32_t*)x, length >> 2);
    offset = length & ~(uintptr_t)0b11;
    x += offset;
    length -= offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
    sum += cnt_first_alleles16((uint16_t*)x, length >> 1);
    offset = length & ~(uintptr_t)0b1;
    x += offset;
    length -= offset;
#endif

    sum += cnt_first_alleles8(x, length);
    return sum;
}

static size_t cnt_second_alleles(const uint8_t* x, size_t num_cols) {
    uintptr_t offset;
    size_t length = get_alleles_length_from_num_cols(num_cols);
    size_t sum = 0;

    if (length == 0) return 0;

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
    offset = (uintptr_t)x & 0b1;
    sum += cnt_second_alleles8(x, offset);
    length -= offset;
    x += offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
    offset = (uintptr_t)x & 0b10;
    if (length >= offset) {
        sum += cnt_second_alleles16((uint16_t*)x, offset >> 1);
        length -= offset;
        x += offset;
    }
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 64 == 0)
    offset = (uintptr_t)x & 0b100;
    if (length >= offset) {
        sum += cnt_second_alleles32((uint32_t*)x, offset >> 2);
        length -= offset;
        x += offset;
    }

    sum += cnt_second_alleles64((uint64_t*)x, length >> 3);
    offset = length & ~(uintptr_t)0b111;
    x += offset;
    length -= offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
    sum += cnt_second_alleles32((uint32_t*)x, length >> 2);
    offset = length & ~(uintptr_t)0b11;
    x += offset;
    length -= offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
    sum += cnt_second_alleles16((uint16_t*)x, length >> 1);
    offset = length & ~(uintptr_t)0b1;
    x += offset;
    length -= offset;
#endif

    sum += cnt_second_alleles8(x, length);
    return sum;
}

static void flip_alleles(uint8_t* x, size_t num_cols) {
    uintptr_t offset;
    size_t length = num_cols >> 2;
    size_t frac = num_cols - (length << 2);
    if (frac > 0) length++;
    if (length == 0) return;

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
    offset = (uintptr_t)x & 0b1;
    flip_alleles8(x, offset);
    length -= offset;
    x += offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
    offset = (uintptr_t)x & 0b10;
    if (length >= offset) {
        flip_alleles16((uint16_t*)x, offset >> 1);
        length -= offset;
        x += offset;
    }
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 64 == 0)
    offset = (uintptr_t)x & 0b100;
    if (length >= offset) {
        flip_alleles32((uint32_t*)x, offset >> 2);
        length -= offset;
        x += offset;
    }

    offset = length & ~(uintptr_t)0b111;
    flip_alleles64((uint64_t*)x, length >> 3);
    x += offset;
    length -= offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
    offset = length & ~(uintptr_t)0b11;
    flip_alleles32((uint32_t*)x, length >> 2);
    x += offset;
    length -= offset;
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
    offset = length & ~(uintptr_t)0b1;
    flip_alleles16((uint16_t*)x, length >> 1);
    x += offset;
    length -= offset;
#endif

    flip_alleles8(x, length);
    x += length - 1;

    switch (frac) {
        case 3:
            *x = *x & 0b00111111;
            break;
        case 2:
            *x = *x & 0b00001111;
            break;
        case 1:
            *x = *x & 0b00000011;
            break;
        default:
            break;
    }
}

pio_status_t
libplinkio_flip_alleles_(libplinkio_loci_private_t loci, struct pio_bed_file_t* bed_file, size_t num_samples)
{
    int fd = -1;
    void* mapped_file = NULL;
    size_t snps_length = 0;
    libplinkio_mmap_state_private_t mmap_state = {0};
    size_t num_loci = libplinkio_get_num_loci_(loci);

    fflush(bed_file->fp);
    fd = fileno(bed_file->fp);
    if(fd < 0) goto error;

    mapped_file = libplinkio_mmap_(fd, LIBPLINKIO_MMAP_READWRITE_, &mmap_state);
    if (mapped_file == NULL) goto error;

    struct bed_header_t header = bed_header_init2( num_loci, num_samples, mapped_file );
    size_t num_rows = bed_header_num_rows( &header );
    size_t num_cols = bed_header_num_cols( &header );
    uint8_t* snps = (uint8_t*)mapped_file + bed_header_data_offset( &header );

    snps_length = get_alleles_length_from_num_cols(num_cols);
    for (size_t i = 0; i < num_rows; i++) {
        uint8_t* cur_snps = snps + snps_length*i;
        struct pio_locus_t* cur_locus = libplinkio_get_locus_(loci, i);
        if (cnt_first_alleles(cur_snps, num_cols) > cnt_second_alleles(cur_snps, num_cols)) {
            char* tmp_allele = cur_locus->allele1;
            cur_locus->allele1 = cur_locus->allele2;
            cur_locus->allele2 = tmp_allele;
            flip_alleles(cur_snps, num_cols);
        }
    }

    /* Release alloacted resources */
    if (libplinkio_munmap_(mapped_file, &mmap_state) != 0) {
        mapped_file = NULL;
        goto error;
    }

    return PIO_OK;

error:
    if (mapped_file != NULL) libplinkio_munmap_(mapped_file, &mmap_state);
    return PIO_ERROR;
}

