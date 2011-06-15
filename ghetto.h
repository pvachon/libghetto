/*
 * Main header for libghetto - the cheap, low-level TIFF structure
 * interpretation library. This library does not make an effort to 
 * interpret anything. Really, it's only useful for accessing TIFF
 * data in strangely structured files (like camera RAW files).
 */
#ifndef __INCLUDE_GHETTO_H__
#define __INCLUDE_GHETTO_H__

/* TODO: wrap these in some testing macros */
typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef unsigned short      UINT16;
typedef short               INT16;

/* The opaque TIFF handle used for all ghetto operations */
typedef struct tiff     tiff_t;
typedef int             TIFF_STATUS;

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

/* TIFF datatypes */
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

/* Open the TIFF file */
TIFF_STATUS tiff_open(tiff_t **fp, const char *file, const char *mode);

/* Close the TIFF file */
TIFF_STATUS tiff_close(tiff_t *fp);

/*******************************************************************/
/* Functions for managing the IFDs                                 */
/*******************************************************************/

/* Get the offset of the root IFD */
TIFF_STATUS tiff_get_base_ifd_offset(tiff_t *fp, tiff_off_t *off);

/* Read a TIFF IFD at offset */
TIFF_STATUS tiff_read(tiff_t *fp, tiff_off_t off);

/* Get the count of tags in the active IFD */
TIFF_STATUS tiff_get_ifd_tag_count(tiff_t *fp);

/* Get information about a given tag */
TIFF_STATUS tiff_get_tag_info(tiff_t *fp, tiff_tag_id_t tag_id,
                              tiff_tag_t **tag_info);

/*******************************************************************/
/* Functions for managing a TIFF tag                               */
/*******************************************************************/

/* Get the type and count of the tag (one of TIFF_TYPE_*) */
TIFF_STATUS tiff_get_tag_type(tiff_t *fp, tiff_tag_t *tag_info,
                              int *type, int *size);

/* Get the actual data associated with the tag. Must be an array that
 * is sizeof(type) * size bytes. Use tiff_get_tag_type to find out this
 * information.
 */
TIFF_STATUS tiff_get_tag_data(tiff_t *fp, tiff_tag_t *tag_info,
                              void *data);

/* Clean up a tiff_tag_t */
TIFF_STATUS tiff_free_tag_info(tiff_t *fp, tiff_tag_t *tag_info);


#endif /* __INCLUDE_GHETTO_H__ */

