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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

TIFF_STATUS tiff_get_base_ifd_offset(tiff_t *fp, tiff_off_t *off)
{
    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(off);

    *off = fp->root_ifd;

    return TIFF_OK;
}

static TIFF_STATUS tiff_ingest_ifd(tiff_t *fp, tiff_ifd_t *ifd,
                                   void *buf, size_t entries)
{
    uint8_t *buf_off = (uint8_t *)buf;
    int i;

    TIFF_ASSERT_ARG(ifd);
    TIFF_ASSERT_ARG(buf);

    TIFF_ASSERT(entries != 0);

    ifd->tags = (tiff_tag_t *)calloc(1, sizeof(tiff_tag_t) * entries);

    TIFF_TRACE("Opening IFD with %zd entries\n", entries);

    if (ifd->tags == NULL) {
        TIFF_TRACE("Failed to allocate %zd bytes for tag info\n",
            sizeof(tiff_tag_t) * (size_t)entries);
        return TIFF_NO_MEMORY;
    }

    TIFF_TRACE("{ %-8s %-8s %-8s %-8s }\n",
            "ID", "type", "count", "value");

    /* Start parsing the IFD records */
    for (i = 0; i < (size_t)entries; i++) {
        uint64_t val;
        tiff_tag_id_t tag_id;
        uint32_t count;
        uint16_t type;

        tag_id = TIFF_WORD(buf_off, IFD_ENTRY_TAG, fp->endianess);
        type = TIFF_WORD(buf_off, IFD_ENTRY_TYPE, fp->endianess);
        count = TIFF_DWORD(buf_off, IFD_ENTRY_COUNT, fp->endianess);
        /* The value in the field is stored unswapped. This is because
         * this value can represent either an offset into the file, or
         * actual data.
         */
        val = TIFF_DWORD(buf_off, IFD_ENTRY_OFFSET, MACH_ENDIANESS);

        TIFF_TRACE("{ %8.8u %8.4u %-8.8x %-8.8x }\n",
            (unsigned)tag_id, (unsigned)type, (unsigned)count,
            (unsigned)val);

        ifd->tags[i].id = tag_id;
        ifd->tags[i].type = (int)type;
        ifd->tags[i].count = count;
        ifd->tags[i].offset = val;

        buf_off += IFD_ENTRY_LEN;
    }

    return TIFF_OK;
}

TIFF_STATUS tiff_make_ifd(tiff_t *fp, void *buf, size_t count, tiff_ifd_t **ifd)
{
    tiff_ifd_t *new_ifd = NULL;
    uint16_t dir_ents = 0;
    TIFF_STATUS ret;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(buf);
    TIFF_ASSERT_ARG(ifd);

    *ifd = NULL;

    /* Make sure the count allows for at least one IFD entry. */
    TIFF_ASSERT(count >= (2 + IFD_ENTRY_LEN + sizeof(uint32_t)));

    /* Read the first WORD to figure out how big the IFD is */
    dir_ents = TIFF_WORD(buf, 0, fp->endianess);

    if (dir_ents == 0 || (dir_ents * IFD_ENTRY_LEN > count)) {
        TIFF_TRACE("IFD data is too small for given number of entries\n");
        return TIFF_RANGE_ERROR;
    }

    new_ifd = (tiff_ifd_t *)calloc(1, sizeof(tiff_ifd_t));
    if (new_ifd == NULL) {
        return TIFF_NO_MEMORY;
    }

    /* Get the offset to the next IFD */
    new_ifd->next_ifd_off = TIFF_DWORD(buf, 2 + ((size_t)dir_ents * IFD_ENTRY_LEN),
        fp->endianess);
    new_ifd->tag_count = dir_ents;

    TIFF_TRACE("next IFD offset: %08x\n", (unsigned)new_ifd->next_ifd_off);

    /* Now ingest the IFD */
    if ( (ret = tiff_ingest_ifd(fp, new_ifd, ((uint8_t *)buf) + 2, dir_ents)) 
        != TIFF_OK)
    {
        free(new_ifd);
        return ret;
    }

    *ifd = new_ifd;

    return TIFF_OK;
}

/* Read a TIFF IFD at offset */
TIFF_STATUS tiff_read_ifd(tiff_t *fp, tiff_off_t off, tiff_ifd_t **ifd)
{
    tiff_ifd_t *new_ifd;
    uint16_t dir_ents;
    size_t count, bytes;
    uint8_t *buf = NULL;

    TIFF_STATUS ret = TIFF_OK;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);

    *ifd = NULL;

    TIFF_SEEK(fp, off, TIFF_SEEK_SET);
    if (TIFF_READ(fp, 2, 1, &dir_ents, &count) != TIFF_OK) {
        TIFF_TRACE("read %zd entries, aborting\n", count);
        return TIFF_END_OF_FILE;
    }

    dir_ents = TIFF_SWAP_WORD(dir_ents, fp->endianess);

    if (dir_ents < 1) {
        TIFF_TRACE("zero entries found in IFD!\n");
        return TIFF_RANGE_ERROR;
    }

    TIFF_PRINT("ifd @ %08x - %d entries\n", (unsigned)off, (int)dir_ents);

    /* load up a buffer of count IFD entries + 4 bytes for the next IFD offset */
    bytes = (size_t)dir_ents * IFD_ENTRY_LEN + 4;

    buf = (uint8_t *)calloc(1, bytes);

    if (buf == NULL) {
        TIFF_TRACE("Failed to allocate %zd bytes\n", bytes);
        return TIFF_NO_MEMORY;
    }

    if (TIFF_READ(fp, bytes, 1, buf, &count) != TIFF_OK) {
        TIFF_TRACE("Failed to read %zd bytes", bytes);
        ret = TIFF_END_OF_FILE;
        goto done_free;
    }

    /* Allocate new IFD */
    new_ifd = (tiff_ifd_t *)calloc(1, sizeof(tiff_ifd_t));

    if (new_ifd == NULL) {
        TIFF_TRACE("Failed to allocate %zd bytes for IFD\n", sizeof(tiff_ifd_t));
        ret = TIFF_NO_MEMORY;
        goto done_free;
    }

    /* Grab the offset of the next IFD */
    new_ifd->next_ifd_off = TIFF_DWORD(buf, (size_t)dir_ents * IFD_ENTRY_LEN,
        fp->endianess);

    new_ifd->tag_count = (size_t)dir_ents;

    TIFF_TRACE("Next IFD: %08x\n", (unsigned)new_ifd->next_ifd_off);

    if ( (ret = tiff_ingest_ifd(fp, new_ifd, buf, dir_ents)) != TIFF_OK ) {
        goto done_free_ifd;
    }

    free(buf);

    *ifd = new_ifd;

    return TIFF_OK;

done_free_ifd:
    if (new_ifd) free(new_ifd);

done_free:
    if (buf) free(buf);

    return ret;
}

TIFF_STATUS tiff_get_next_ifd_offset(tiff_t *fp, tiff_ifd_t *ifd, tiff_off_t *off)
{
    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);
    TIFF_ASSERT_ARG(off);

    *off = ifd->next_ifd_off;

    return TIFF_OK;
}

/* Get the count of tags in the provided IFD */
TIFF_STATUS tiff_get_ifd_tag_count(tiff_t *fp, tiff_ifd_t *ifd, size_t *count)
{
    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);
    TIFF_ASSERT_ARG(count);

    *count = ifd->tag_count;

    return TIFF_OK;
}

static TIFF_STATUS tiff_find_tag(tiff_ifd_t *ifd, tiff_tag_id_t tag_id,
                                 tiff_tag_t **tag_info)
{
    int i;
    for (i = 0; i < ifd->tag_count; i++) {
        tiff_tag_t *tag = &ifd->tags[i];

        if (tag->id == tag_id) {
            *tag_info = tag;
            TIFF_TRACE("Found tag at offset %d\n", i);
            return TIFF_OK;
        }
    }
    TIFF_TRACE("Tag with ID %d not found\n", (int)tag_id);
    return TIFF_TAG_NOT_FOUND;
}

/* Get information about a given tag */
TIFF_STATUS tiff_get_tag(tiff_t *fp,
                         tiff_ifd_t *ifd,
                         tiff_tag_id_t tag_id,
                         tiff_tag_t **tag_info)
{
    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);
    TIFF_ASSERT_ARG(tag_info);

    *tag_info = NULL;

    return tiff_find_tag(ifd, tag_id, tag_info);
}

/* Get tag by index in IFD */
TIFF_STATUS tiff_get_tag_indexed(tiff_t *fp, tiff_ifd_t *ifd, size_t index,
                                 tiff_tag_t **tag_info)
{
    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);
    TIFF_ASSERT_ARG(tag_info);

    if (index > ifd->tag_count) {
        TIFF_TRACE("Index %zd is out of range.\n", index);
        return TIFF_RANGE_ERROR;
    }

    *tag_info = &ifd->tags[index];

    return TIFF_OK;
}

TIFF_STATUS tiff_free_ifd(tiff_t *fp, tiff_ifd_t *ifd)
{
    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);

    if (ifd->tags) {
        memset(ifd->tags, 0, ifd->tag_count * IFD_ENTRY_LEN);
        free(ifd->tags);
    }

    memset(ifd, 0, sizeof(tiff_ifd_t));
    free(ifd);

    return TIFF_OK;
}

