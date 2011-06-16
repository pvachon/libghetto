#include <ghetto.h>
#include <ghetto_fp.h>
#include <ghetto_priv.h>

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Check if this is a TIFF image. If so, gather relevant information to
 * interpret the image's structure.
 */
static TIFF_STATUS tiff_is_tiff_file(tiff_t *fp)
{
    uint8_t header[TIFF_HEADER_LEN];
    size_t count = 0;
    uint16_t magic = 0;

    TIFF_SEEK(fp, 0, TIFF_SEEK_SET);
    TIFF_READ(fp, TIFF_HEADER_LEN, 1, header, &count);

    if (count < 1) {
        return TIFF_NOT_TIFF;
    }

    if (header[0] == ENDIANESS_INTEL && header[1] == ENDIANESS_INTEL) {
        fp->endianess = ENDIAN_LITTLE;
    } else if (header[0] == ENDIANESS_MOTOROLA && header[1] == ENDIANESS_MOTOROLA) {
        fp->endianess = ENDIAN_BIG;
    } else {
        return TIFF_NOT_TIFF;
    }

    magic = TIFF_WORD(header, TIFF_HEADER_MAGIC, fp->endianess);

    if (magic != TIFF_MAGIC) {
        TIFF_TRACE("magic = %04x\n", magic);
        return TIFF_NOT_TIFF;
    }

    fp->root_ifd = TIFF_DWORD(header, TIFF_HEADER_IFD, fp->endianess);

    TIFF_TRACE("root_ifd = %08x\n", (unsigned)fp->root_ifd);

    return TIFF_OK;
}

TIFF_STATUS tiff_open_ex(tiff_t **fp, tiff_file_mgr_t *mgr,
                         const char *file, const char *mode)
{
    tiff_t *fptr = NULL;
    TIFF_STATUS ret = TIFF_OK;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(mgr);
    TIFF_ASSERT_ARG(file);
    TIFF_ASSERT_ARG(mode);

    *fp = NULL;

    fptr = (tiff_t *)calloc(1, sizeof(tiff_t));
    if (fptr == NULL) {
        ret = TIFF_NO_MEMORY;
        goto fail;
    }

    fptr->mgr = mgr;
    if ( (ret = fptr->mgr->open(&fptr->fp, file, mode)) != TIFF_OK ) {
        goto fail_free_fptr;
    }

    /* Try to identify the file as a TIFF file */
    if ( (ret = tiff_is_tiff_file(fptr)) != TIFF_OK ) {
        goto fail_free_fptr;
    }

    *fp = fptr;
    return TIFF_OK;

fail_free_fptr:
    free(fptr);

fail:
    return ret;
}

TIFF_STATUS tiff_open(tiff_t **fp, const char *file, const char *mode)
{
    return tiff_open_ex(fp, tiff_stdio_mgr, file, mode);
}

TIFF_STATUS tiff_close(tiff_t *fp)
{
    TIFF_ASSERT_ARG(fp);

    fp->mgr->close(fp->fp);

    memset(fp, 0, sizeof(tiff_t));

    free(fp);

    return TIFF_OK;
}

