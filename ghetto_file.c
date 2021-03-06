/*
  Copyright (c) 2011, Phil Vachon <phil@cowpig.ca>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  - Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

  - Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
        goto fail_close_file;
    }

    *fp = fptr;
    return TIFF_OK;

fail_close_file:
    fptr->mgr->close(fptr->fp);

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

TIFF_STATUS tiff_read(tiff_t *fp, size_t offset, size_t size, size_t nmemb,
                      void *dest_buf, size_t *count)
{
    TIFF_STATUS ret;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(dest_buf);

    if (size * nmemb == 0) {
        return TIFF_RANGE_ERROR;
    }

    if ( (ret = TIFF_SEEK(fp, offset, TIFF_SEEK_SET) ) != TIFF_OK ) {
        return ret;
    }

    return TIFF_READ(fp, size, nmemb, dest_buf, count);
}

