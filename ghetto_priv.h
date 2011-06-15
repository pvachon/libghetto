#ifndef __INCLUDE_GHETTO_PRIV_H__
#define __INCLUDE_GHETTO_PRIV_H__

#include <ghetto.h>
#include <ghetto_fp.h>

#include <stdio.h>

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
};

struct tiff_tag {
    tiff_off_t offset;
    size_t count;
    tiff_tag_id_t type;
};

TIFF_STATUS tiff_free_ifd(struct tiff_ifd *ifd);

#define TIFF_ASSERT_ARG(x) \
    if ((x) == NULL) { \
        return TIFF_BAD_ARGUMENT; \
    }


#endif /* __INCLUDE_GHETTO_PRIV_H__ */

