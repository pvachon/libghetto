#ifndef __INCLUDE_GHETTO_FP_H__
#define __INCLUDE_GHETTO_FP_H__

#include <ghetto.h>

#include <stdio.h>
#include <stdint.h>

struct tiff_file_hdl;
typedef struct tiff_file_hdl tiff_file_hdl_t;

/* Where to seek from for tiff_file_mgr::seek */
#define TIFF_SEEK_CUR   0
#define TIFF_SEEK_SET   1
#define TIFF_SEEK_END   2

typedef struct tiff_file_mgr {
    TIFF_STATUS (*open)(tiff_file_hdl_t **hdl, const char *file, const char *mode);
    TIFF_STATUS (*close)(tiff_file_hdl_t *hdl);
    TIFF_STATUS (*read)(tiff_file_hdl_t *hdl, size_t size, size_t nmemb, void *buf,
                        size_t *count);
    TIFF_STATUS (*seek)(tiff_file_hdl_t *hdl, size_t offset, int whence);
} tiff_file_mgr_t;

/* Default stdio/native I/O based file manager */
extern tiff_file_mgr_t *tiff_stdio_mgr;

/* Extended TIFF open. Allows setting a non-standard I/O strategy */
TIFF_STATUS tiff_open_ex(tiff_t **fp, tiff_file_mgr_t *mgr,
                         const char *file, const char *mode);

#endif /* __INCLUDE_GHETTO_FP_H__ */

