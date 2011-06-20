#include <ghetto.h>

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define TIFF_TAG_SUBIFD     330
#define TIFF_TAG_EXIFIFD    34665

const char *type_string[] = {
    "unknown",
    "byte",
    "ascii",
    "short",
    "long",
    "rational",
    "signed byte",
    "undefined (byte)",
    "signed short",
    "signed long",
    "signed rational",
    "float",
    "double"
};

static void dump_16(uint8_t *bytes, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (i % 8 == 0 && !(i % 16 == 0)) {
			printf("-%02x", (unsigned int)bytes[i]);
		} else {
			printf(" %02x", (unsigned int)bytes[i]);
		}
	}

    if (count < 16) {
        for (i = 0; i < (16 - count); i++) {
            printf("   ");
        }
    }

	printf("   |");
	for (i = 0; i < count; i++) {
		if (isprint(bytes[i])) printf("%c", bytes[i]);
		else printf(".");
	}
	printf("|\n");
}

static void dump_buffer(void *bytes, int count)
{
    int i, off = 0;

    uint8_t *buf = (uint8_t*)bytes;

    if (buf == NULL) {
        printf("Attempted to print a NULL buffer\n");
        return;
    }

    for (i = 0; i + 16 < count; i += 16) {
        printf("%08x: ", (unsigned int)off);
        dump_16(&buf[i], 16);
        off+=16;

    }

    if (count % 16) {
        printf("%08x: ", (unsigned int)off);
        dump_16(&buf[(count / 16) * 16], count % 16);
    }
}

void print_unknown_tag(tiff_t *fp, tiff_tag_t *tag)
{
    int id, count, type;

    tiff_get_tag_info(fp, tag, &id, &type, &count);

    printf("%6.6d %4.4d %6.6d\n",
        id, type, count);
}

void print_ascii(tiff_t *fp, tiff_ifd_t *ifd, tiff_tag_t *tag)
{
    int id, count, type;
    char *value = NULL;

    tiff_get_tag_info(fp, tag, &id, &type, &count);

    value = (char *)calloc(1, count + 1);

    tiff_get_tag_data(fp, ifd, tag, value);

    printf("%6.6d %4.4d %6.6d \"%s\"\n",
        id, type, count, value);

    free(value);
}

void display_ifd(tiff_t *fp, tiff_ifd_t *ifd)
{
    size_t i, tag_count;
    tiff_tag_t *tag;

    tiff_get_ifd_tag_count(fp, ifd, &tag_count);

    if (tag_count == 0) { return; }

    printf("%6.6s %4.4s %6.6s %8.8s\n",
        "tag", "type", "count", "data_value");
    printf("==========================================================\n");

    for (i = 0; i < tag_count; i++) {
        int type = 0;
        tiff_get_tag_indexed(fp, ifd, i, &tag);
        tiff_get_tag_info(fp, tag, NULL, &type, NULL);

        switch (type) {
        case TIFF_TYPE_ASCII:
            print_ascii(fp, ifd, tag);
            break;
        default:
            print_unknown_tag(fp, tag);
            break;
        }
    }
}

int dump_tag_data(tiff_t *fp, tiff_ifd_t *ifd, tiff_tag_t *tag)
{
    int count, type, id, ifd_count;
    tiff_ifd_t *new_ifd;
    tiff_off_t offset = 0;

    uint8_t *data = NULL;
    uint8_t *ifd_off;

    tiff_get_tag_info(fp, tag, &id, &type, &count);

    ifd_count = count;

    printf("Tag: %d (%d items of type %s)\n",
        id, count, type_string[type]);

    data = (uint8_t *)calloc(1, tiff_get_type_size(type) * count);

    if (data == NULL) {
        printf("Failed to allocate %zd bytes\n",
            tiff_get_type_size(type) * count);
        return -1;
    }

    tiff_get_tag_data(fp, ifd, tag, data);

    /* dump_buffer(data, count * tiff_get_type_size(type)); */
    ifd_off = data;

    if (!strncmp(data, "Nikon", 5)) {
        /* Read the Nikon IFD */
        ifd_off += 18;
        ifd_count -= 18;
        tiff_get_raw_tag_field(fp, tag, &offset);
        offset += 18;
    } else if (!strncmp(data, "AOC", 3)) {
        printf("Not sure about Pentax MakerNote...\n");
        goto done;
    } else {
        printf("Assuming Canon MakerNote...\n");
    }

    printf("Dumping MakerNote IFD\n");
    tiff_make_ifd(fp, ifd_off, ifd_count, offset, &new_ifd);

    display_ifd(fp, new_ifd);

    tiff_free_ifd(fp, new_ifd);

done:
    if (data) free(data);
    return 0;
}

int find_and_dump_tag(tiff_t *fp, tiff_ifd_t *ifd, tiff_tag_id_t tagid)
{
    tiff_tag_t *ifd_tag;

    tiff_get_tag(fp, ifd, tagid, &ifd_tag);

    if (ifd_tag == NULL) {
        printf("Tag %d not found!\n", (int)tagid);
        return -1;
    }

    dump_tag_data(fp, ifd, ifd_tag);

    return 0;
}

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

    tiff_get_tag_data(fp, parent, ifd_tag, tag_data);

    for (i = 0; i < tag_count; i++) {
        uint32_t ifd_off = tag_data[i];
        tiff_ifd_t *subifd_ptr;

        printf("Opening IFD at %08x\n", ifd_off);
        tiff_read_ifd(fp, ifd_off, &subifd_ptr);

        display_ifd(fp, subifd_ptr);

        /* Dump any MakerNote tags we find */
        find_and_dump_tag(fp, subifd_ptr, 37500);

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
        display_ifd(fp, ifd);

        read_ifd(fp, ifd, TIFF_TAG_SUBIFD);
        read_ifd(fp, ifd, TIFF_TAG_EXIFIFD);

        tiff_get_next_ifd_offset(fp, ifd, &ifd_off);
        tiff_free_ifd(fp, ifd);
    } while (ifd_off != 0);

    tiff_close(fp);

    return 0;
}

