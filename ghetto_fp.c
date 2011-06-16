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
#include <ghetto_priv.h>
#include <ghetto_fp.h>

#include <stdio.h>

TIFF_STATUS tiff_stdio_open(tiff_file_hdl_t **hdl, const char *file, const char *mode)
{
    FILE *fp = NULL;

    TIFF_ASSERT_ARG(hdl);
    TIFF_ASSERT_ARG(file);
    TIFF_ASSERT_ARG(mode);

    fp = fopen(file, mode);

    if (fp == NULL) {
        return TIFF_FILE_NOT_FOUND;
    }

    *hdl = (void *)fp;
    return TIFF_OK;
}

TIFF_STATUS tiff_stdio_close(tiff_file_hdl_t *hdl)
{
    FILE *fp = NULL;

    TIFF_ASSERT_ARG(hdl);

    fp = (FILE *)fp;

    fclose(fp);

    return TIFF_OK;
}

TIFF_STATUS tiff_stdio_read(tiff_file_hdl_t *hdl,
                            size_t size, size_t nmemb, void *buf, size_t *count)
{
    FILE *fp = NULL;
    size_t rd_cnt;

    TIFF_ASSERT_ARG(hdl);
    TIFF_ASSERT_ARG(buf);

    fp = (FILE *)hdl;

    rd_cnt = fread(buf, size, nmemb, fp);

    if (count) {
        *count = rd_cnt;
    }

    return TIFF_OK;
}

TIFF_STATUS tiff_stdio_seek(tiff_file_hdl_t *hdl, size_t offset, int whence)
{
    FILE *fp = NULL;
    int sk_whence = 0;

    TIFF_ASSERT_ARG(hdl);

    switch (whence) {
    case TIFF_SEEK_CUR:
        sk_whence = SEEK_CUR;
        break;
    case TIFF_SEEK_SET:
        sk_whence = SEEK_SET;
        break;
    case TIFF_SEEK_END:
        sk_whence = SEEK_END;
        break;
    default:
        return TIFF_RANGE_ERROR;
    }

    fp = (FILE *)hdl;

    if (fseek(fp, offset, sk_whence)) {
        return TIFF_END_OF_FILE;
    }

    return TIFF_OK;
}

tiff_file_mgr_t tiff_stdio_mgr_s = {
    .open = tiff_stdio_open,
    .close = tiff_stdio_close,
    .read = tiff_stdio_read,
    .seek = tiff_stdio_seek
};

tiff_file_mgr_t *tiff_stdio_mgr = &tiff_stdio_mgr_s;

