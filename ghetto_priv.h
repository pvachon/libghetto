#ifndef __INCLUDE_GHETTO_PRIV_H__
#define __INCLUDE_GHETTO_PRIV_H__

#include <ghetto.h>
#include <ghetto_fp.h>

#include <stdio.h>
#include <stdlib.h>

#define ENDIAN_BIG      0x0
#define ENDIAN_LITTLE   0x1

struct tiff {
    tiff_file_hdl_t *fp;

    int endianess;
    tiff_off_t root_ifd;
    tiff_file_mgr_t *mgr;
};

struct tiff_tag;

struct tiff_ifd {
    struct tiff *fp;
    struct tiff_tag *tags;
    size_t tag_count;
    tiff_off_t next_ifd_off;
};

struct tiff_tag {
    tiff_tag_id_t id;
    int type;
    size_t count;
    tiff_off_t offset;
};

/* Helper Macros */

#ifdef _DEBUG
#define TIFF_PRINT(x, ...) \
    printf("ghetto: " x, ##__VA_ARGS__)
#else
#define TIFF_PRINT(...)
#endif

#define TIFF_TRACE(x, ...) \
    TIFF_PRINT("%s@%d: " x, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef _DEBUG
#define TIFF_ASSERT(x) \
    if (!(x)) { \
        TIFF_TRACE("assertion failure: " #x "\n"); \
        exit(-1); \
    }
#else
#define TIFF_ASSERT(...)
#endif

#define TIFF_ASSERT_RETURN(x, r) \
    { \
        TIFF_STATUS __ret; \
        if ((__ret = (x)) != TIFF_OK) {\
            TIFF_TRACE("call failed: " #x " returned %d\n", (int)__ret); \
            return (r); \
        } \
    }

#define TIFF_ASSERT_ARG(x) \
    if ((x) == NULL) { \
        TIFF_TRACE("argument is NULL: " #x "\n"); \
        return TIFF_BAD_ARGUMENT; \
    }

#define TIFF_READ(fp, size, nmemb, buf, count) \
    fp->mgr->read(fp->fp, size, nmemb, buf, count)

#define TIFF_SEEK(fp, off, whence) \
    fp->mgr->seek(fp->fp, off, whence)

static inline uint16_t tiff_swap_word(uint16_t word)
{
    return (uint16_t)((((word) >> 8) & 0xff) | ((word) & 0xff) << 8);
}

#define TIFF_SWAP_WORD(word, endianess) \
    (((endianess) != MACH_ENDIANESS) ? \
        (uint16_t)((((word) >> 8) & 0xff) | ((word) & 0xff) << 8) : (word))

static inline uint32_t tiff_swap_dword(uint32_t word)
{
    return (uint32_t)(((uint32_t)((word >> 24) & 0xff)) |
           ((uint32_t)((word >> 8) & 0xff00)) |
           ((uint32_t)((word & 0xff) << 24)) |
           ((uint32_t)((word & 0xff00) << 8)));
}

#define TIFF_SWAP_DWORD(word, endianess) \
    (((endianess) != MACH_ENDIANESS) ? \
        tiff_swap_dword(word) : (word))

/* Macros for reading bytes/words/dwords from a buffer */
#define TIFF_BYTE(buf, bytes_off) \
    ((uint8_t *)buf)[bytes_off]

#define TIFF_WORD(buf, bytes_off, endianess) \
    TIFF_SWAP_WORD(*((uint16_t *)(((uint8_t *)buf) + (bytes_off))), endianess)

#define TIFF_DWORD(buf, bytes_off, endianess) \
    TIFF_SWAP_DWORD(*((uint32_t *)(((uint8_t *)buf) + (bytes_off))), endianess)

/* Defines related to image structure/offsets/etc */
#define TIFF_HEADER_LEN     8
#define TIFF_HEADER_ENDIAN  0
#define TIFF_HEADER_MAGIC   2
#define TIFF_HEADER_IFD     4

#define ENDIANESS_INTEL     'I'
#define ENDIANESS_MOTOROLA  'M'
#define TIFF_MAGIC          42

/* IFD-related defines */
#define IFD_ENTRY_LEN       12
#define IFD_ENTRY_TAG       0
#define IFD_ENTRY_TYPE      2
#define IFD_ENTRY_COUNT     4
#define IFD_ENTRY_OFFSET    8

#define TIFF_TAG_DATA_FIELD_SIZE    0x4

#endif /* __INCLUDE_GHETTO_PRIV_H__ */

