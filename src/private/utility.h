#ifndef INCLUDED_PLINKIO_PRIVATE_UTILITY_H_
#define INCLUDED_PLINKIO_PRIVATE_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <limits.h>
#include <stdlib.h>

#ifndef PLINKIO_PTR_BIT_

#if (UINTPTR_MAX == UINTMAX_C(0xffff))
#define PLINKIO_PTR_BIT_ 16
#elif (UINTPTR_MAX == UINTMAX_C(0xffffffff))
#define PLINKIO_PTR_BIT_ 32
#elif (UINTPTR_MAX == UINTMAX_C(0xffffffffffffffff))
#define PLINKIO_PTR_BIT_ 64
#else
#define PLINKIO_PTR_BIT_ 0
#endif

#endif

#ifndef FORCE_INLINE
#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) 
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif
#endif

#define UNUSED_PARAM(x) ((void)(x))

typedef enum {
    LIBPLINKIO_MMAP_READONLY_,
    LIBPLINKIO_MMAP_READWRITE_,
    LIBPLINKIO_MMAP_NONE_
} libplinkio_mmap_mode_private_t;

typedef struct {
#ifdef _WIN32
    HANDLE file_mapping_handle;
#else
    size_t st_size;
#endif
} libplinkio_mmap_state_private_t;

static FORCE_INLINE uint8_t libplinkio_popcnt8_(uint8_t x);
#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
static FORCE_INLINE uint16_t libplinkio_popcnt16_(uint16_t x);
#endif
#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
static FORCE_INLINE uint32_t libplinkio_popcnt32_(uint32_t x);
#endif
#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 64 == 0)
static FORCE_INLINE uint64_t libplinkio_popcnt64_(uint64_t x);
#endif

static FORCE_INLINE size_t libplinkio_bits_msb_size_(size_t x);

int libplinkio_get_random_(uint8_t* buffer, size_t length);
int libplinkio_get_random_32_(char* buffer, size_t length);

int libplinkio_tmp_open_(const char* filename_prefix, const size_t filename_prefix_length);

void* libplinkio_mmap_(int fd, libplinkio_mmap_mode_private_t mode, libplinkio_mmap_state_private_t* state);
int libplinkio_munmap_(void* mapped_file, libplinkio_mmap_state_private_t* state);

int libplinkio_ftruncate_(int fd, size_t size);

int libplinkio_change_mode_and_open_(int fd, int flags);

static FORCE_INLINE uint8_t libplinkio_popcnt8_(uint8_t x) {
    x = (x & 0x55) + (x >> 1 & 0x55);
    x = (x & 0x33) + (x >> 2 & 0x33);
    return (x & 0x0f) + (x >> 4 & 0x0f);
}

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 16 == 0)
static FORCE_INLINE uint16_t libplinkio_popcnt16_(uint16_t x) {
    x = (x & 0x5555) + (x >> 1 & 0x5555);
    x = (x & 0x3333) + (x >> 2 & 0x3333);
    x = (x & 0x0f0f) + (x >> 4 & 0x0f0f);
    return (x & 0x00ff) + (x >> 8 & 0x00ff);
}
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 32 == 0)
static FORCE_INLINE uint32_t libplinkio_popcnt32_(uint32_t x) {
    x = (x & 0x55555555) + (x >> 1 & 0x55555555);
    x = (x & 0x33333333) + (x >> 2 & 0x33333333);
    x = (x & 0x0f0f0f0f) + (x >> 4 & 0x0f0f0f0f);
    x = (x & 0x00ff00ff) + (x >> 8 & 0x00ff00ff);
    return (x & 0x0000ffff) + (x >> 16 & 0x0000ffff);
}
#endif

#if (PLINKIO_PTR_BIT_ != 0 && PLINKIO_PTR_BIT_ % 64 == 0)
static FORCE_INLINE uint64_t libplinkio_popcnt64_(uint64_t x) {
    x = (x & 0x5555555555555555) + (x >> 1 & 0x5555555555555555);
    x = (x & 0x3333333333333333) + (x >> 2 & 0x3333333333333333);
    x = (x & 0x0f0f0f0f0f0f0f0f) + (x >> 4 & 0x0f0f0f0f0f0f0f0f);
    x = (x & 0x00ff00ff00ff00ff) + (x >> 8 & 0x00ff00ff00ff00ff);
    x = (x & 0x0000ffff0000ffff) + (x >> 16 & 0x0000ffff0000ffff);
    return (x & 0x00000000ffffffff) + (x >> 32 & 0x00000000ffffffff);
}
#endif

static FORCE_INLINE size_t libplinkio_bits_msb_size_(size_t x)
{
#if SIZE_MAX == UINTMAX_C(0xFFFF)
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    return x ^ (x >> 1);
#elif SIZE_MAX == UINTMAX_C(0xFFFFFFFF)
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x ^ (x >> 1);
#elif SIZE_MAX == UINTMAX_C(0xFFFFFFFFFFFFFFFF)
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    return x ^ (x >> 1);
#else
    size_t tmp = x;
    int bits = -1;
    while (tmp != 0)
    {
        tmp /= 2;
        bits++;
    }
    if (bits != -1) {
        return (size_t)1 << bits;
    } else {
        return 0;
    }
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* End of INCLUDED_PLINKIO_PRIVATE_UTILITY_H_ */
