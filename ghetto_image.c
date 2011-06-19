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

/* Baseline tags defining the final image characteristics */
#define TIFF_TAG_IMAGEWIDTH         256
#define TIFF_TAG_IMAGELENGTH        257
#define TIFF_TAG_BITSPERSAMPLE      258
#define TIFF_TAG_COMPRESSION        259
#define TIFF_TAG_SAMPLESPERPIXEL    277
#define TIFF_TAG_PLANARCONFIG       284

/* Strip-related tags */
#define TIFF_TAG_STRIPOFFSETS       273
#define TIFF_TAG_ROWSPERSTRIP       278
#define TIFF_TAG_STRIPBYTECOUNTS    279

/* Tile-related tags */
#define TIFF_TAG_TILEWIDTH          322
#define TIFF_TAG_TILEHEIGHT         323
#define TIFF_TAG_TILEOFFSETS        324
#define TIFF_TAG_TILEBYTECOUNTS     325

/* Sample Format (extended sample types) */
#define TIFF_TAG_SAMPLEFORMAT       339

/* It is possible to have an IFD that doesn't contain image data (i.e.
 * an EXIF IFD. As such, try to detect if an IFD contains imagery.
 */
static TIFF_STATUS tiff_is_image_ifd(tiff_t *fp, tiff_ifd_t *ifd)
{
    TIFF_STATUS ret;
    tiff_tag_t *tag;
    /* To check if this IFD contains an image, try to find the 
     * ImageWidth and ImageLength tags.
     */

    if ( (ret = tiff_get_tag(fp, ifd, TIFF_TAG_IMAGEWIDTH, &tag)) != TIFF_OK )
    {
        return ret;
    }

    if ( (ret = tiff_get_tag(fp, ifd, TIFF_TAG_IMAGELENGTH, &tag)) != TIFF_OK )
    {
        return ret;
    }


    return TIFF_OK;
}

TIFF_STATUS tiff_get_image_attribs(tiff_t *fp, tiff_ifd_t *ifd,
                                   unsigned *width, unsigned *height,
                                   unsigned *samples)
{
    tiff_tag_t *tag = NULL;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);

    if (tiff_is_image_ifd(fp, ifd)) {
        return TIFF_IFD_NOT_IMAGE;
    }

    if (width) {
        TIFF_ASSERT_RETURN(tiff_get_tag(fp, ifd, TIFF_TAG_IMAGEWIDTH, &tag),
                TIFF_IFD_NOT_IMAGE);
        TIFF_ASSERT_RETURN(tiff_get_tag_data(fp, tag, width),
                TIFF_IFD_NOT_IMAGE);
    }

    if (height) {
        TIFF_ASSERT_RETURN(tiff_get_tag(fp, ifd, TIFF_TAG_IMAGELENGTH, &tag),
                TIFF_IFD_NOT_IMAGE);
        TIFF_ASSERT_RETURN(tiff_get_tag_data(fp, tag, height),
                TIFF_IFD_NOT_IMAGE);
    }

    if (samples) {
        TIFF_ASSERT_RETURN(tiff_get_tag(fp, ifd, TIFF_TAG_SAMPLESPERPIXEL, &tag),
                TIFF_IFD_NOT_IMAGE);
        TIFF_ASSERT_RETURN(tiff_get_tag_data(fp, tag, samples),
                TIFF_IFD_NOT_IMAGE);
    }

    return TIFF_OK;
}

TIFF_STATUS tiff_get_image_sample_info(tiff_t *fp, tiff_ifd_t *ifd,
                                       int *bits, int *data_type)
{
    tiff_tag_t *tag = NULL;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);

    if (tiff_is_image_ifd(fp, ifd)) {
        return TIFF_IFD_NOT_IMAGE;
    }

    if (bits) {
        TIFF_ASSERT_RETURN(tiff_get_tag(fp, ifd, TIFF_TAG_BITSPERSAMPLE, &tag),
            TIFF_IFD_NOT_IMAGE);
        TIFF_ASSERT_RETURN(tiff_get_tag_data(fp, tag, bits),
            TIFF_IFD_NOT_IMAGE);
    }

    if (data_type) {
        if (tiff_get_tag(fp, ifd, TIFF_TAG_SAMPLEFORMAT, &tag) != TIFF_OK) {
            *data_type = TIFF_SAMPLEFORMAT_UINT;
        } else {
            TIFF_ASSERT_RETURN(tiff_get_tag_data(fp, tag, bits),
                TIFF_IFD_NOT_IMAGE);
        }
    }

    return TIFF_OK;
}

TIFF_STATUS tiff_get_image_structure(tiff_t *fp, tiff_ifd_t *ifd,
                                     int *tile_count,
                                     int *tile_width, int *tile_height,
                                     unsigned *compression)
{
    tiff_tag_t *tag = NULL;

    TIFF_ASSERT_ARG(fp);
    TIFF_ASSERT_ARG(ifd);

    if (tiff_is_image_ifd(fp, ifd)) {
        return TIFF_IFD_NOT_IMAGE;
    }

    if (tile_count) *tile_count = 0;
    if (tile_width) *tile_width = 0;
    if (tile_height) *tile_height = 0;

    if (compression) {
        TIFF_ASSERT_RETURN(tiff_get_tag(fp, ifd, TIFF_TAG_COMPRESSION, &tag),
            TIFF_IFD_NOT_IMAGE);
        TIFF_ASSERT_RETURN(tiff_get_tag_data(fp, tag, compression),
            TIFF_IFD_NOT_IMAGE);
    }

    return TIFF_OK;
}

