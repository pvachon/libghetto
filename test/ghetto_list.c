#include <ghetto.h>

#include <stdio.h>
#include <stdint.h>

#define TIFF_TAG_SUBIFD     330
#define TIFF_TAG_EXIFIFD    34665

int read_ifd(tiff_t *fp, tiff_ifd_t *parent, int tag_id)
{
    tiff_off_t ifd_off;
    tiff_tag_t *ifd_tag;

    int tag_count;
    int tag_type;

    uint32_t *tag_data = NULL;
    int i;

    tiff_get_tag(fp, parent, tag_id, &ifd_tag);

    if (ifd_tag == NULL) {
        printf("Tag %d not found in IFD!\n", tag_id);
        return -1;
    }

    tiff_get_tag_info(fp, ifd_tag, NULL, &tag_type, &tag_count);

    if (tag_count < 1) {
        printf("Tag count is 0. Malformed TIFF file!\n");
        return -1;
    }

    tag_data = (uint32_t *)malloc(sizeof(uint32_t) * tag_count);

    if (tag_data == NULL) {
        perror("malloc");
        exit(-1);
    }

    tiff_get_tag_data(fp, ifd_tag, tag_data);

    for (i = 0; i < tag_count; i++) {
        uint32_t ifd_off = tag_data[i];
        tiff_ifd_t *subifd_ptr;

        printf("Opening IFD at %08x\n", ifd_off);
        tiff_read_ifd(fp, ifd_off, &subifd_ptr);

        if (subifd_ptr != NULL) tiff_free_ifd(fp, subifd_ptr);
    }

    free(tag_data);
}

int main(int argc, const char *argv[])
{
    tiff_t *fp;
    TIFF_STATUS ret;
    tiff_off_t ifd_off;
    tiff_ifd_t *ifd;

    tiff_tag_t *subifd;
    int subifd_count = 0;
    int subifd_type = 0;
    uint32_t *subifds = NULL;

    int i;

    if (argc < 2) {
        printf("insufficient arguments.\n");
        return -1;
    }

    ret = tiff_open(&fp, argv[1], "r");

    if (ret != TIFF_OK) {
        printf("file could not be openened: %d\n", ret);
        return -1;
    }

    tiff_get_base_ifd_offset(fp, &ifd_off);

    do {
        tiff_read_ifd(fp, ifd_off, &ifd);

        read_ifd(fp, ifd, TIFF_TAG_SUBIFD);
        read_ifd(fp, ifd, TIFF_TAG_EXIFIFD);

        tiff_get_next_ifd_offset(fp, ifd, &ifd_off);
        tiff_free_ifd(fp, ifd);
    } while (ifd_off != 0);

    tiff_close(fp);

    return 0;
}

