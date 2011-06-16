#include <ghetto.h>

#include <stdio.h>
#include <stdint.h>

#define TIFF_TAG_SUBIFD     330

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

    tiff_read_ifd(fp, ifd_off, &ifd);

    /* Get SubIFDs tag/tag info */
    tiff_get_tag(fp, ifd, TIFF_TAG_SUBIFD, &subifd);

    tiff_get_tag_info(fp, subifd, NULL, &subifd_type, &subifd_count);


    if (subifd_count > 0) {
        printf("Reading %d SubIFDs\n", subifd_count);
        subifds = (uint32_t *)malloc(sizeof(uint32_t) * subifd_count);
        tiff_get_tag_data(fp, subifd, subifds);

        for (i = 0; i < subifd_count; i++) {
            tiff_ifd_t *subifd_ptr;

            printf("Opening SubIFD at %08x\n", subifds[i]);
            tiff_read_ifd(fp, subifds[i], &subifd_ptr);


            tiff_free_ifd(fp, subifd_ptr);
        }
        free(subifds);
    } else {
        printf("No SubIFDs!\n");
    }

    tiff_free_ifd(fp, ifd);

    tiff_close(fp);

    return 0;
}

