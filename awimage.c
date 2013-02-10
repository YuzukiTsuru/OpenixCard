/*
 * awimage.c, pack and unpack Allwinner A10 images.
 * Copyright (c) 2012, Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and COPYING for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sys/stat.h>

#include "twofish.h"
#include "rc6.h"

#include "imagewty.h"

#undef TF_DECRYPT_WORKING

enum {
	OUTPUT_IMGREPACKER,
	OUTPUT_UNIMG,
};

int	flag_encryption_enabled,
	flag_compat_output,
	flag_verbose;

/* Crypto */
rc6_ctx_t header_ctx;
rc6_ctx_t fileheaders_ctx;
rc6_ctx_t filecontent_ctx;
u4byte tf_key[32];

const char *progname;

static void
recursive_mkdir(const char *dir)
{
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);

	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for(p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	}

	mkdir(tmp, S_IRWXU);
}

static void
crypto_init(void)
{
    char key[32];
    int i;

    /* Initialize RC6 context for header */
    memset(key, 0, sizeof(key));
    key[sizeof(key)-1] = 'i';
    rc6_init(key, sizeof(key) * 8, &header_ctx);

    /* Initialize RC6 context for fileheaders */
    memset(key, 1, sizeof(key));
    key[sizeof(key)-1] = 'm';
    rc6_init(key, sizeof(key) * 8, &fileheaders_ctx);

    /* Initialize RC6 context for file content */
    memset(key, 2, sizeof(key));
    key[sizeof(key)-1] = 'g';
    rc6_init(key, sizeof(key) * 8, &filecontent_ctx);

    /* Initialize TwoFish key for file content of non-fex files */
    tf_key[0] = 5;
    tf_key[1] = 4;
    for (i = 2; i < 32; i++)
        tf_key[i] = tf_key[i-2] + tf_key[i-1];
}

static void *rc6_decrypt_inplace(void *p, size_t len, rc6_ctx_t *ctx)
{
    int i;

    /* If encryption is disabled, we've got nothing to do */
    if (!flag_encryption_enabled)
        return p + len;

    for (i = 0; i < len/16; i++) {
        rc6_dec(p, ctx);
        p += 16;
    }

    return p;
}

#if TF_DECRYPT_WORKING
static void *tf_decrypt_inplace(void *p, size_t len)
{
    int i;

    /* If encryption is disabled, we've got nothing to do */
    if (!flag_encryption_enabled)
        return p + len;

    tf_init(tf_key, 128);

    for (i = 0; i < len/16; i++) {
        tf_decrypt(p, p);
        p += 16;
    }

    return p;
}
#endif

static FILE*
dir_fopen(const char *dir, const char *path, const char *mode)
{
    char outfn[512];
    char *p;
    int len;

    strcpy(outfn, "./");
    strcat(outfn, dir);
    len = strlen(outfn);
    if (outfn[len-1] != '/' && path[0] != '/')
        strcat(outfn, "/");
    strcat(outfn, path);

    /* If there's a directory path in there, create it */
    p = strrchr(outfn, '/');
    if (*p) {
        *p = '\0';
        recursive_mkdir(outfn);
        *p = '/';
    }

    return fopen(outfn, mode);
}

static int
pack_image(const char *indn, const char *outfn)
{
    return 0;
}

static int
unpack_image(const char *infn, const char *outdn)
{
    int num_files, pid, vid, hardware_id, firmware_id;
    struct imagewty_header *header;
    FILE *ifp, *lfp, *ofp;
    void *image, *curr;
    long imagesize;
    int i;

    ifp = fopen(infn, "rb");
    if (ifp == NULL) {
        fprintf(stderr, "Error: unable to open %s!\n", infn);
        return 2;
    }

    fseek(ifp, 0, SEEK_END);
    imagesize = ftell(ifp);
    fseek(ifp, 0, SEEK_SET);

    if (imagesize <= 0) {
        fprintf(stderr, "Error: Invalid file size %ld (%s)\n",
            imagesize, strerror(errno));
        return 3;
    }

    image = malloc(imagesize);
    if (!image) {
        fprintf(stderr, "Error: Unable to allocate memory for image: %ld\n", imagesize);
        return 4;
    }

    fread(image, imagesize, 1, ifp);
    fclose(ifp);

    /* Check for encryption; see bug #2 (A31 unencrypted images) */
    header = (struct imagewty_header*)image;
    if (memcmp(header->magic, IMAGEWTY_MAGIC, IMAGEWTY_MAGIC_LEN) == 0)
        flag_encryption_enabled = 0;

    /* Decrypt header (padded to 1024 bytes) */
    curr = rc6_decrypt_inplace(image, 1024, &header_ctx);


    /* Check version of header and setup our local state */
    if (header->header_version == 0x0300) {
        num_files = header->v3.num_files;
        hardware_id = header->v3.hardware_id;
        firmware_id = header->v3.firmware_id;
        pid = header->v3.pid;
        vid = header->v3.vid;
    } else /*if (header->header_version == 0x0100)*/ {
        num_files = header->v1.num_files;
        hardware_id = header->v1.hardware_id;
        firmware_id = header->v1.firmware_id;
        pid = header->v1.pid;
        vid = header->v1.vid;
    }

    /* Decrypt file headers */
    curr = rc6_decrypt_inplace(curr, num_files * 1024, &fileheaders_ctx);

    /* Decrypt file contents */
    for (i=0; i < num_files; i++) {
        struct imagewty_file_header *filehdr;
	uint64_t stored_length;
	const char *filename;
        void *next;

        filehdr = (struct imagewty_file_header*)(image + 1024 + (i * 1024));
        if (header->header_version == 0x0300) {
            stored_length = filehdr->v3.stored_length;
            filename = filehdr->v3.filename;
        } else {
            stored_length = filehdr->v1.stored_length;
            filename = filehdr->v1.filename;
        }

        next = rc6_decrypt_inplace(curr, stored_length, &filecontent_ctx);
#if TF_DECRYPT_WORKING
        if (!(strlen(filehdr->filename) >= 4 &&
            strncmp(filehdr->filename + strlen(filehdr->filename) -4, ".fex", 4) == 0)) {
            /* Not a 'FEX' file, so we need to decrypt it even more! */
            tf_decrypt_inplace(curr, filehdr->stored_length);
        }
#endif
        curr = next;
    }

    if (flag_compat_output == OUTPUT_UNIMG) {
        lfp = dir_fopen(outdn, "base.hdr", "wb");
        if (lfp) {
            uint32_t *hdr = image + IMAGEWTY_MAGIC_LEN;
            fprintf(lfp, "%.8s\r\n", header->magic);
            for (i = 0; i < (sizeof(*header) - IMAGEWTY_MAGIC_LEN) / sizeof(uint32_t); i++)
                fprintf(lfp, "%08X\r\n", hdr[i]);
            fclose(lfp);
        }

        lfp = dir_fopen(outdn, "Filelist.txt", "wb");
    } else if (flag_compat_output == OUTPUT_IMGREPACKER) {
        lfp = dir_fopen(outdn, "image.cfg", "wb");
        if (lfp != NULL) {
            char timestr[256];
            struct tm *tm;
            time_t t;
            time(&t);
            tm = localtime(&t);
            strcpy(timestr, asctime(tm));
            /* strip newline */
            timestr[strlen(timestr) -1] = '\0';

            fputs(";/**************************************************************************/\r\n", lfp);
            fprintf(lfp, "; %s\r\n", timestr);
            fprintf(lfp, "; generated by %s\r\n", progname);
            fprintf(lfp, "; %s\r\n", infn);
            fputs(";/**************************************************************************/\r\n", lfp);
            fputs("[FILELIST]\r\n", lfp);
        }
    }

    for (i=0; i < num_files; i++) {
        uint64_t stored_length, original_length;
        struct imagewty_file_header *filehdr;
        char hdrfname[32], contfname[32];
        const char *filename;
        uint64_t offset;

        filehdr = (struct imagewty_file_header*)(image + 1024 + (i * 1024));
        if (header->header_version == 0x0300) {
            stored_length = filehdr->v3.stored_length;
            original_length = filehdr->v3.original_length;
            filename = filehdr->v3.filename;
            offset = filehdr->v3.offset;
        } else {
            stored_length = filehdr->v1.stored_length;
            original_length = filehdr->v1.original_length;
            filename = filehdr->v1.filename;
            offset = filehdr->v1.offset;
        }

        if (flag_compat_output == OUTPUT_UNIMG) {
            printf("Extracting: %.8s %.16s (%Lu, %Lu)\n", 
                filehdr->maintype, filehdr->subtype,
                original_length, stored_length);

            sprintf(hdrfname, "%.8s_%.16s.hdr", filehdr->maintype, filehdr->subtype);
            ofp = dir_fopen(outdn, hdrfname, "wb");
            if (ofp) {
                fwrite(filehdr, filehdr->total_header_size, 1, ofp);
                fclose(ofp);
            }

            sprintf(contfname, "%.8s_%.16s", filehdr->maintype, filehdr->subtype);
            ofp = dir_fopen(outdn, contfname, "wb");
            if (ofp) {
                fwrite(image + offset, original_length, 1, ofp);
                fclose(ofp);
            }

            fprintf(lfp, "%s\t%s\r\n", hdrfname, contfname);
        } else if (flag_compat_output == OUTPUT_IMGREPACKER) {
	    printf("Extracting %s\n", filename);

            ofp = dir_fopen(outdn, filename, "wb");
            if (ofp) {
                fwrite(image + offset, original_length, 1, ofp);
                fclose(ofp);
            }

            fprintf(lfp, "\t{filename = INPUT_DIR .. \"%s\", maintype = \"%.8s\", subtype = \"%.16s\",},\r\n",
                filename[0] == '/' ? filename+1 : filename,
                filehdr->maintype, filehdr->subtype);
        }
    }

    if (lfp && flag_compat_output == OUTPUT_IMGREPACKER) {
        /* Now print the relevant stuff for the image.cfg */
        fputs("\r\n[IMAGE_CFG]\r\n", lfp);
        fprintf(lfp, "version = 0x%06x\r\n", header->version);
        fprintf(lfp, "pid = 0x%08x\r\n", pid);
        fprintf(lfp, "vid = 0x%08x\r\n", vid);
        fprintf(lfp, "hardwareid = 0x%03x\r\n", hardware_id);
        fprintf(lfp, "firmwareid = 0x%03x\r\n", firmware_id);
        fprintf(lfp, "imagename = %s\r\n", infn);
        fputs("filelist = FILELIST\r\n", lfp);
    }

    if (lfp)
        fclose(lfp);

    return 0;
}

int
main(int argc, char **argv)
{
    struct stat statbuf;
    char outfn[512];
    char *out, *in;
    int c, rc;

    /* Generate program name from argv[0] */
    progname = argv[0];
    if (progname[0] == '.' && progname[1] == '/')
        progname += 2;

    /* assume we're encrypted */
    flag_encryption_enabled = 1;

    /* Setup default output format */
    flag_compat_output = OUTPUT_UNIMG;

    /* Now scan the cmdline options */
    do {
        c = getopt(argc, argv, "vurh");
        if (c == EOF)
            break;
        switch (c) {
        case 'u':
            flag_compat_output = OUTPUT_UNIMG;
            break;
        case 'r':
            flag_compat_output = OUTPUT_IMGREPACKER;
            break;
        case 'v':
            flag_verbose++;
            break;
        case 'h':
            fprintf(stderr, "%s [-vurh] {image dir|dir image}\n"
                    "  -r         imgRepacker compatibility\n"
                    "  -u         unimg.exe compatibility\n"
                    "  -v         Be verbose\n"
                    "  -h         Print help\n", argv[0]);
            return -1;
        case '?':
            fprintf(stderr, "%s: invalid option -%c\n",
                argv[0], optopt);
            return 1;
        }
    } while (1);

    if (argc - optind > 2) {
        fprintf(stderr, "%s: extra arguments\n", argv[0]);
        return 1;
    } else if (argc - optind < 1) {
        fprintf(stderr, "%s: missing file argument\n", argv[0]);
        return 1;
    }

    /* If we get here, we have a file spec and possibly options */
    crypto_init();   

    rc = stat(argv[optind], &statbuf);
    if (rc) {
        fprintf(stderr, "%s: cannot stat '%s'!\n", argv[0], argv[optind]);
        return 1;
    }

    out = (argc - optind) == 2 ? argv[optind+1] : NULL;
    in = argv[optind];

    if (S_ISDIR(statbuf.st_mode)) {
        /* We're packing, lets see if we have to generate the output image filename ourselfs */
        if (out == NULL) {
            int len = strlen(in);
            strcpy(outfn, in);
            if (in[len-1] == '/') len--;
            printf("%s\n", in+len-5);
            if (len > 5 && strncmp(in+len-5, ".dump", 5) == 0)
		outfn[len-5] = '\0';
            else {
                outfn[len] = '\0';
            	strcat(outfn, ".img");
            }

            out = outfn;
        }
        fprintf(stderr, "%s: packing %s into %s\n", argv[0], in, out);
        pack_image(in, out);
    } else {
        /* We're unpacking, lets see if we have to generate the output directory name ourself */
        if (out == NULL) {
            strcpy(outfn, in);
            strcat(outfn, ".dump");
        } else {
            strcpy(outfn, out);
	}
	out = outfn;

        fprintf(stderr, "%s: unpacking %s to %s\n", argv[0], in, out);
	unpack_image(in, out);
    }

    return 0;
}
