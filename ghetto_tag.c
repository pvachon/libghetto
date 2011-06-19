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

static size_t tiff_type_sizes[] = {
    0,
    1, /* Byte */
    1, /* ASCII */
    2, /* Short */
    4, /* Long */
    8, /* Rational */
    1, /* SByte */
    1, /* Undefined */
    2, /* SShort */
    4, /* SLong */
    8, /* SRational */
    4, /* Float */
    8  /* Double */
};

static void tiff_swap_dword_buffer(void *buf, size_t count, int endianess)
{
    size_t i;
    uint32_t *dwbuf = (uint32_t *)buf;

    if (endianess == MACH_ENDIANESS) return;

    for (i = 0; i < count; i++) {
        dwbuf[i] = tiff_swap_dword(dwbuf[i]);
    }
}

static void tiff_swap_word_buffer(void *buf, size_t count, int endianess)
{
    size_t i;
    uint16_t *wbuf = (uint16_t *)buf;

    if (endianess == MACH_ENDIANESS) return;

    for (i = 0; i < count; i++) {
        wbuf[i] = tiff_swap_word(wbuf[i]);
    }
}

size_t tiff_get_type_size(int type)
{
    if (type < TIFF_TYPE_BYTE || type > TIFF_TYPE_DOUBLE) {
        return 0;
    }

    return tiff_type_sizes[type];
}

TIFF_STATUS tiff_get_tag_info(tiff_t *fp, tiff_tag_t *tag_info,
                              int *id, int *type, int *count)
{
    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(tag_info);

    if (id) *id = tag_info->id;
    if (type) *type = tag_info->type;
    if (count) *count = tag_info->count;

    return TIFF_OK;
}

TIFF_STATUS tiff_get_tag_data(tiff_t *fp, tiff_tag_t *tag_info,
                              void *data)
{
    size_t tag_size = 0, count = 0;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(tag_info);
    TIFF_ASSERT_ARG(data);

    if ((tag_size = tiff_get_type_size(tag_info->type)) == 0) {
        return TIFF_UNKNOWN_TYPE;
    }

    if (tag_size * tag_info->count <= TIFF_TAG_DATA_FIELD_SIZE) {
        /* Extract the data from the field itself */
        memcpy(data, &tag_info->offset, tag_size * tag_info->count);
    } else {
        if (tag_info->offset == 0) {
            TIFF_TRACE("Malformed tag %d - zero offset into file\n",
                tag_info->id);
            return TIFF_TAG_MALFORMED;
        }
        /* Read data from file, into provided buffer. We are treating
         * tag_info->offset as an offset here, so we swap it to native
         * endianess, treating it as a DWORD.
         */
        TIFF_SEEK(fp, TIFF_SWAP_DWORD(tag_info->offset, fp->endianess),
            TIFF_SEEK_SET);
        TIFF_READ(fp, tag_size, tag_info->count, data, &count);

        if (count < tag_info->count) {
            return TIFF_END_OF_FILE;
        }
    }

    /* Byte swap buffer of data */
    if (tag_size == 2) {
        tiff_swap_word_buffer(data, tag_info->count, fp->endianess);
    } else if (tag_size == 4) {
        tiff_swap_dword_buffer(data, tag_info->count, fp->endianess);
    } else if (tag_size == 8) {
        if (tag_info->type == TIFF_TYPE_RATIONAL ||
            tag_info->type == TIFF_TYPE_SRATIONAL)
        {
            tiff_swap_dword_buffer(data, tag_info->count * 2, fp->endianess);
        } else {
            TIFF_TRACE("Unknown/unhandled data type for byte swapping. Be careful!\n");
       }
    }

    return TIFF_OK;
}

TIFF_STATUS tiff_free_tag_info(tiff_t *fp, tiff_tag_t *tag_info)
{
    return TIFF_OK;
}

