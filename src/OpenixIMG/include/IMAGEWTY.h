/*
 * IMAGEWTY.h Allwinner IMAGEWTY Handler
 * Copyright (c) 2012, Ithamar R. Adema <ithamar@upgrade-android.com>
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXIMG_IMAGEWTY_H
#define OPENIXIMG_IMAGEWTY_H

#include <stdint.h>

#define IMAGEWTY_MAGIC        "IMAGEWTY"
#define IMAGEWTY_MAGIC_LEN    8

struct imagewty_header {
    char magic[IMAGEWTY_MAGIC_LEN];
    uint32_t header_version;    /* Image header version (seen values are 0x0100 / 0x0300) */
    uint32_t header_size;        /* Image header size (sizeof(struct imagewty_header)) */
    uint32_t ram_base;
    uint32_t version;        /* format version (IMAGEWTY_VERSION) */
    uint32_t image_size;            /* total size of image file (rounded up to 256 bytes?) */
    uint32_t image_header_size;     /* image header size (including padding) */
    union {
        struct {
            uint32_t pid;            /* USB peripheral ID (from image.cfg) */
            uint32_t vid;            /* USB vendor ID (from image.cfg) */
            uint32_t hardware_id;        /* Hardware ID (from image.cfg) */
            uint32_t firmware_id;        /* Firmware ID (from image.cfg) */
            uint32_t val1;            /* */
            uint32_t val1024;        /* */
            uint32_t num_files;        /* Total number of files embedded */
            uint32_t val1024_2;        /* */
            uint32_t val0;            /* */
            uint32_t val0_2;        /* */
            uint32_t val0_3;        /* */
            uint32_t val0_4;        /* */
            /* 0x0050 */
        } v1;
        struct {
            uint32_t unknown;
            uint32_t pid;            /* USB peripheral ID (from image.cfg) */
            uint32_t vid;            /* USB vendor ID (from image.cfg) */
            uint32_t hardware_id;        /* Hardware ID (from image.cfg) */
            uint32_t firmware_id;        /* Firmware ID (from image.cfg) */
            uint32_t val1;            /* */
            uint32_t val1024;        /* */
            uint32_t num_files;        /* Total number of files embedded */
            uint32_t val1024_2;        /* */
            uint32_t val0;            /* */
            uint32_t val0_2;        /* */
            uint32_t val0_3;        /* */
            uint32_t val0_4;        /* */
            /* 0x0060 */
        } v3;
    };
};

#define IMAGEWTY_FHDR_MAINTYPE_LEN    8
#define IMAGEWTY_FHDR_SUBTYPE_LEN    16
#define IMAGEWTY_FHDR_FILENAME_LEN    256

struct imagewty_file_header {
    uint32_t filename_len;
    uint32_t total_header_size;
    const char maintype[IMAGEWTY_FHDR_MAINTYPE_LEN];
    const char subtype[IMAGEWTY_FHDR_SUBTYPE_LEN];
    union {
        struct {
            uint32_t unknown_3;
            uint32_t stored_length;
            uint32_t original_length;
            uint32_t offset;
            uint32_t unknown;
            const char filename[IMAGEWTY_FHDR_FILENAME_LEN];
        } v1;
        struct {
            uint32_t unknown_0;
            const char filename[IMAGEWTY_FHDR_FILENAME_LEN];
            uint32_t stored_length;
            uint32_t pad1;
            uint32_t original_length;
            uint32_t pad2;
            uint32_t offset;
        } v3;
    };
};

#endif /* IMAGEWTY_H */

