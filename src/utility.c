#include "private/utility.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

// libplinkio_get_random_()
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#include <share.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#endif

// libplinkio_mmap_read() libplinkio_munmap_()
#include <fcntl.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#ifdef _WIN32
#include <inttypes.h>
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#include <sys/types.h>

#ifndef _WIN32
#define LIBPLINKIO_FD_STR_MAX_LENGTH_ 20
#endif

int libplinkio_get_random_(uint8_t* buffer, size_t length)
{
    if (length > 256) goto error;
#ifdef _WIN32
    if (
        FAILED(HRESULT_FROM_NT(BCryptGenRandom(
            NULL, (PUCHAR)buffer, (ULONG)length, BCRYPT_USE_SYSTEM_PREFERRED_RNG
        )))
    ) goto error;
#elif (__unix__) || (defined(__APPLE__) && defined(__MACH__))
    do {
        FILE* fp = fopen("/dev/urandom", "rb");
        if (fp == NULL) goto other_unix_error;
        if (fread(buffer, sizeof(char), length, fp) != length) goto other_unix_error;
        fclose(fp);
        fp = NULL;
        break;

    other_unix_error:
        if (fp != NULL) fclose(fp);
        fp = NULL;
        goto error;
    } while (0);
#else
#error "Random number generator is unavailable."
#endif
    return 0;
error:
    return -1;
}

int libplinkio_get_random_32_(char* buffer, size_t length) {
    const char* const conv_table = "0123456789abcdefghijklmnopqrstuv";
    for (size_t i = 0; i < length; i++) {
        uint8_t random_num = 0;
        if (libplinkio_get_random_(&random_num, 1) != 0) goto error;
        buffer[i] = conv_table[random_num & 0b11111];
    }
    return 0;

error:
    return -1;
}


int libplinkio_tmp_open_(const char* filename_prefix, const size_t filename_prefix_length)
{
    int fd = -1;

    const size_t random_length = 12;
    const size_t filename_length = filename_prefix_length + random_length + 1;

    char* filename = (char*)calloc(filename_length + 1, sizeof(char));
    if (filename == NULL) goto error;

    for (;;) {
        strncpy(filename, filename_prefix, filename_prefix_length);
        filename[filename_prefix_length] = '.';
        for (size_t i = 0; i < random_length; i++) {
            if (libplinkio_get_random_32_(filename + filename_prefix_length + i + 1, 1) != 0) goto error;
        }
        filename[filename_length] = '\0';

#ifdef _WIN32
        errno_t err_open = _sopen_s(
            &fd,
            filename,
            _O_BINARY | _O_CREAT | _O_TEMPORARY | _O_EXCL | _O_NOINHERIT | _O_RDWR,
            _SH_DENYRW,
            _S_IREAD | _S_IWRITE
        );
        if (err_open != 0) {
            if (err_open != EEXIST) goto error;
            continue;
        }
#else
        fd = open(
            filename,
            O_CREAT | O_EXCL | O_RDWR,
            S_IRUSR | S_IWUSR
        );
        if (fd == -1) {
            if (errno != EEXIST) goto error;
            continue;
        }
#endif
        if(unlink(filename) != 0) goto error;
        break;
    }

    free(filename);
    return fd;

error:
    if(fd != -1) close(fd);
    free(filename);
    return -1;
}

void* libplinkio_mmap_(int fd, libplinkio_mmap_mode_private_t mode, libplinkio_mmap_state_private_t* state) {
    struct stat file_stats;
    void* mapped_file = NULL;
    libplinkio_mmap_state_private_t state_init = { 0 };
    *state = state_init;
    if( fstat( fd, &file_stats ) == -1 ) goto error;
#ifdef _WIN32
    DWORD page_protect_mode = PAGE_NOACCESS;
    DWORD desired_access = FILE_MAP_READ;
    if (mode == LIBPLINKIO_MMAP_READONLY_) {
        page_protect_mode = PAGE_READONLY;
        desired_access = FILE_MAP_READ;
    } else if (mode == LIBPLINKIO_MMAP_READWRITE_) {
        page_protect_mode = PAGE_READWRITE;
        desired_access = FILE_MAP_WRITE;
    } else {
        goto error;
    }
    state->file_mapping_handle = CreateFileMapping(
        (HANDLE)_get_osfhandle(fd),
        NULL,
        page_protect_mode,
        0, 0,
        NULL
    );
    if(state->file_mapping_handle == NULL) goto error;
    mapped_file = MapViewOfFile(
        state->file_mapping_handle,
        desired_access,
        0, 0,
        file_stats.st_size
    );
    if(mapped_file == NULL) goto error;
#else
    state->st_size = file_stats.st_size;
    int prot = PROT_NONE;
    if (mode == LIBPLINKIO_MMAP_READONLY_) {
        prot = PROT_READ;
    } else if (mode == LIBPLINKIO_MMAP_READWRITE_) {
        prot = PROT_READ | PROT_WRITE;
    } else {
        goto error;
    }
    mapped_file = mmap(
        NULL,
        file_stats.st_size,
        prot,
        MAP_FILE | MAP_PRIVATE,
        fd,
        0
    );
    if( mapped_file == MAP_FAILED ) goto error;
#endif
    return mapped_file;

error:
#ifdef _WIN32
    if (mapped_file != NULL) UnmapViewOfFile(mapped_file);
    if (state->file_mapping_handle != NULL) CloseHandle(state->file_mapping_handle);
#else
    if (mapped_file != NULL && mapped_file != MAP_FAILED) munmap(mapped_file, state->st_size);
#endif
    return NULL;
}

int libplinkio_munmap_(void* mapped_file, libplinkio_mmap_state_private_t* state) {
#ifdef _WIN32
    if (UnmapViewOfFile(mapped_file) == 0) goto error;
    if (CloseHandle(state->file_mapping_handle) == 0) goto error;
#else
    if (munmap(mapped_file, state->st_size) != 0) goto error;
#endif
    return 0;

error:
    return -1;
}

int libplinkio_ftruncate_(int fd, size_t size) {
#ifdef _MSC_VER
    if(_chsize_s(fd, size) != 0) return -1;
#else
    if(ftruncate(fd, size) == -1) return -1;
#endif
    return 0;
}

int libplinkio_change_mode_and_open_(int fd, int flags) {
    if (fd < 0) return -1;
#ifdef _WIN32 
    HANDLE fhandle = NULL;
    DWORD desired_access = 0;
    if ((flags & O_RDWR) == O_RDWR) {
        desired_access = GENERIC_READ | GENERIC_WRITE;
    } else if ((flags & O_RDONLY) == O_RDONLY) {
        desired_access = GENERIC_READ;
    } else if ((flags & O_WRONLY) == O_WRONLY) {
        desired_access = GENERIC_WRITE;
    } else {
        return -1;
    }
    if (DuplicateHandle(
        GetCurrentProcess(),
        (HANDLE)_get_osfhandle(fd),
        GetCurrentProcess(),
        &fhandle,
        desired_access,
        FALSE,
        0
    ) == 0) {
        return -1;
    };
    return _open_osfhandle((intptr_t)fhandle, flags);
#else
    const char format[] = "/dev/fd/";
    char fd_str_buffer[LIBPLINKIO_FD_STR_MAX_LENGTH_ + 1] = "\0";
    char fd_path[sizeof(format) + LIBPLINKIO_FD_STR_MAX_LENGTH_] = "\0";

    ssize_t actual_length = snprintf(fd_str_buffer, sizeof(fd_str_buffer), "%d", fd);
    if ((actual_length <= 0) || (actual_length - LIBPLINKIO_FD_STR_MAX_LENGTH_ > 0)) return -1;
    fd_str_buffer[actual_length] = '\0';

    strcat(fd_path, format);
    strcat(fd_path, fd_str_buffer);

    int new_fd = open(fd_path, flags);

    return new_fd;
#endif
}