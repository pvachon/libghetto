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

/* Read a TIFF IFD at offset */
TIFF_STATUS tiff_read_ifd(tiff_t *fp, tiff_off_t off, tiff_ifd_t **ifd)
{
    tiff_ifd_t *new_ifd;
    uint16_t dir_ents;
    size_t count, bytes;
    uint8_t *buf = NULL, *buf_off = NULL;
    int i;

    TIFF_STATUS ret = TIFF_OK;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);

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

    new_ifd->tags = (tiff_tag_t *)calloc(1, sizeof(tiff_tag_t) * (size_t)dir_ents);

    if (new_ifd->tags == NULL) {
        TIFF_TRACE("Failed to allocate %zd bytes for tag info\n",
            sizeof(tiff_tag_t) * (size_t)dir_ents);
        ret = TIFF_NO_MEMORY;
        goto done_free_ifd;
    }

    /* Grab the offset of the next IFD */
    new_ifd->next_ifd_off = TIFF_DWORD(buf, (size_t)dir_ents * IFD_ENTRY_LEN,
        fp->endianess);

    new_ifd->tag_count = (size_t)dir_ents;

    TIFF_TRACE("Next IFD: %08x\n", (unsigned)new_ifd->next_ifd_off);

    buf_off = buf;
    TIFF_TRACE("{ %-8s %-8s %-8s %-8s }\n",
            "ID", "type", "count", "value");

    /* Start parsing the IFD records */
    for (i = 0; i < (size_t)dir_ents; i++) {
        uint64_t val;
        tiff_tag_id_t tag_id;
        uint32_t count;
        uint16_t type;

        tag_id = TIFF_WORD(buf_off, IFD_ENTRY_TAG, fp->endianess);
        type = TIFF_WORD(buf_off, IFD_ENTRY_TYPE, fp->endianess);
        count = TIFF_DWORD(buf_off, IFD_ENTRY_COUNT, fp->endianess);
        val = TIFF_DWORD(buf_off, IFD_ENTRY_OFFSET, fp->endianess);

        TIFF_TRACE("{ %8.8u %8.4u %-8.8x %-8.8x }\n",
            (unsigned)tag_id, (unsigned)type, (unsigned)count,
            (unsigned)val);

        new_ifd->tags[i].id = tag_id;
        new_ifd->tags[i].type = (int)type;
        new_ifd->tags[i].count = count;
        new_ifd->tags[i].offset = val;

        buf_off += IFD_ENTRY_LEN;
    }

    free(buf);

    *ifd = new_ifd;

    return TIFF_OK;
/*
done_free_ifd_tags:
    if (new_ifd->tags) free(new_ifd->tags);
*/
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

