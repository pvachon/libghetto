/*
 * Main header for libghetto - the cheap, low-level TIFF structure
 * interpretation library. This library does not make an effort to 
 * interpret anything. Really, it's only useful for accessing TIFF
 * data in strangely structured files (like camera RAW files).
 */
#ifndef __INCLUDE_GHETTO_H__
#define __INCLUDE_GHETTO_H__

#include <stdlib.h>

/* TODO: wrap these in some testing macros */
typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef unsigned short      UINT16;
typedef short               INT16;

/* The opaque TIFF handle used for all ghetto operations */
typedef struct tiff     tiff_t;
typedef int             TIFF_STATUS;

/* forward declare a few useful structs */
struct tiff_ifd;
struct tiff_tag;

typedef struct tiff_ifd tiff_ifd_t;
typedef struct tiff_tag tiff_tag_t;
typedef UINT64          tiff_off_t;
typedef UINT16          tiff_tag_id_t;

/* TIFF status return codes */
#define TIFF_OK             0x0     /* All is well */
#define TIFF_NOT_TIFF       0x1     /* Not a TIFF image */
#define TIFF_NOT_OPEN       0x2     /* File pointer is not for an open image */
#define TIFF_RANGE_ERROR    0x3     /* A specified value is outside valid range */
#define TIFF_BAD_ARGUMENT   0x4     /* Bad argument - something is NULL */
#define TIFF_TAG_NOT_FOUND  0x5     /* Tag was not found in IFD */
#define TIFF_FILE_NOT_FOUND 0x6     /* File cannot be found */
#define TIFF_END_OF_FILE    0x7     /* End of file was unexpectedly hit */
#define TIFF_NO_MEMORY      0x8     /* A memory allocation failed */
#define TIFF_UNKNOWN_TYPE   0x9     /* The type of data represented is unknown */
#define TIFF_IFD_NOT_IMAGE  0xa     /* The provided IFD is not an image dir */
#define TIFF_TAG_MALFORMED  0xb     /* Tag is illegal by TIFF standard */

/* TIFF tag datatypes */
#define TIFF_TYPE_BYTE       1
#define TIFF_TYPE_ASCII      2
#define TIFF_TYPE_SHORT      3
#define TIFF_TYPE_LONG       4
#define TIFF_TYPE_RATIONAL   5
#define TIFF_TYPE_SBYTE      6
#define TIFF_TYPE_UNDEFINED  7
#define TIFF_TYPE_SSHORT     8
#define TIFF_TYPE_SLONG      9
#define TIFF_TYPE_SRATIONAL 10
#define TIFF_TYPE_FLOAT     11
#define TIFF_TYPE_DOUBLE    12

/* TIFF sample formats */
#define TIFF_SAMPLEFORMAT_UINT           1 /* Unsigned integer */
#define TIFF_SAMPLEFORMAT_INT            2 /* Signed integer */
#define TIFF_SAMPLEFORMAT_IEEEFP         3 /* IEEE 734 Floating Point */
#define TIFF_SAMPLEFORMAT_VOID           4 /* Unknown/No type */
#define TIFF_SAMPLEFORMAT_COMPLEXINT     5 /* Complex Integer */
#define TIFF_SAMPLEFORMAT_COMPLEXIEEEFP  6 /* Complex Float */


/* Open the TIFF file */
TIFF_STATUS tiff_open(tiff_t **fp, const char *file, const char *mode);

/* Close the TIFF file */
TIFF_STATUS tiff_close(tiff_t *fp);

/*******************************************************************/
/* Functions for reading arbitrary data from the file              */
/*******************************************************************/

/* Read an arbitrary block of data from the TIFF file. */
TIFF_STATUS tiff_read(tiff_t *fp, size_t offset, size_t size, size_t nmemb,
                      void *dest_buf, size_t *count);

/*******************************************************************/
/* Functions for managing the IFDs                                 */
/*******************************************************************/

/* Get the offset of the root IFD */
TIFF_STATUS tiff_get_base_ifd_offset(tiff_t *fp, tiff_off_t *off);

/* Read a TIFF IFD at offset */
TIFF_STATUS tiff_read_ifd(tiff_t *fp, tiff_off_t off, tiff_ifd_t **ifd);

/* Get the offset of the next IFD */
TIFF_STATUS tiff_get_next_ifd_offset(tiff_t *fp, tiff_ifd_t *ifd, tiff_off_t *off);

/* Get the count of tags in the provided IFD */
TIFF_STATUS tiff_get_ifd_tag_count(tiff_t *fp, tiff_ifd_t *ifd, size_t *count);

/* Get handle for a given tag */
TIFF_STATUS tiff_get_tag(tiff_t *fp, tiff_ifd_t *ifd, tiff_tag_id_t tag_id,
                         tiff_tag_t **tag_info);

/* Get handle for a tag by index in IFD */
TIFF_STATUS tiff_get_tag_indexed(tiff_t *fp, tiff_ifd_t *ifd, size_t index,
                                 tiff_tag_t **tag_info);

/* Create a tiff_ifd_t from a region of memory. This is useful for when
 * a tag points to a structure that contains an IFD (like a MakerNote),
 * but the IFD is prefixed with non-standard data (or some work needs
 * to be done to determine what offset in the tag the IFD is found at).
 * Allows you to specify an offset from which IFD entry offsets will be
 * considered from - some MakerNotes do this.
 */
TIFF_STATUS tiff_make_ifd(tiff_t *fp, void *buf, size_t count, tiff_off_t tag_off,
                          tiff_ifd_t **ifd);

/* Free an IFD record */
TIFF_STATUS tiff_free_ifd(tiff_t *fp, tiff_ifd_t *ifd);

/*******************************************************************/
/* Functions for managing a TIFF tag                               */
/*******************************************************************/

/* Get the type and count of the tag (one of TIFF_TYPE_*) */
TIFF_STATUS tiff_get_tag_info(tiff_t *fp, tiff_tag_t *tag_info,
                              int *id, int *type, int *count);

/* Get the actual data associated with the tag. Must be an array that
 * is sizeof(type) * size bytes. Use tiff_get_tag_type to find out this
 * information.
 */
TIFF_STATUS tiff_get_tag_data(tiff_t *fp, tiff_ifd_t *ifd, tiff_tag_t *tag_info,
                              void *data);

/* Get the raw contents of the TIFF tag's offset field. You only need
 * to do this to work around busted tag contents that specify offsets
 * relative to the start of the tag, rather than the start of the file.
 */
TIFF_STATUS tiff_get_raw_tag_field(tiff_t *fp, tiff_tag_t *tag_info,
                                   tiff_off_t *data);

/* Clean up a tiff_tag_t */
TIFF_STATUS tiff_free_tag_info(tiff_t *fp, tiff_tag_t *tag_info);

/*******************************************************************/
/* Helper Functions for dealing with Imagery                       */
/*******************************************************************/
/* Note: these functions simply operate on IFDs and typically are 
 * very lightweight to call. No I/O should occurs here.
 * PPV Note:
 * libghetto treats strip-oriented image storage as a special case
 * of tiling, where the tile dimensions are not equal.
 */

/* Get the properties of an image stored in an associated IFD. */
TIFF_STATUS tiff_get_image_attribs(tiff_t *fp, tiff_ifd_t *ifd,
                                   unsigned *width, unsigned *height,
                                   unsigned *channels);

/* Get information about each pixel sample. Returns:
 * - bits per pixel
 * - data type of the pixel
 */
TIFF_STATUS tiff_get_image_sample_info(tiff_t *fp, tiff_ifd_t *ifd,
                                       int *bits, int *data_type);

/* Return information about the image structure. Get the number of
 * strips/tiles per image, the width/height of a strip or tile and
 * the compression mechanism used.
 */
TIFF_STATUS tiff_get_image_structure(tiff_t *fp, tiff_ifd_t *ifd,
                                     int *tile_count,
                                     int *tile_width, int *tile_height,
                                     unsigned *compression);

/*******************************************************************/
/* Helper Functions                                                */
/*******************************************************************/

/* Get the size of a single item of a given type */
size_t tiff_get_type_size(int type);

#endif /* __INCLUDE_GHETTO_H__ */

