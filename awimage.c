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

#include "parsecfg.h"

#include "twofish.h"
#include "rc6.h"

#include "imagewty.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sys/stat.h>

#define TF_DECRYPT_WORKING 0

#ifdef WIN32
	#define MKDIR(p)	mkdir(p)
#else
	#define MKDIR(p)	mkdir(p,S_IRWXU)
#endif

enum {
	OUTPUT_IMGREPACKER,
	OUTPUT_UNIMG,
};

int	flag_encryption_enabled,
	flag_compat_output,
        flag_dump_decrypted,
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
			MKDIR(tmp);
			*p = '/';
		}
	}

	MKDIR(tmp);
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

static void *rc6_encrypt_inplace(void *p, size_t len, rc6_ctx_t *ctx)
{
    int i;

    /* If encryption is disabled, we've got nothing to do */
    if (!flag_encryption_enabled)
        return p + len;

    for (i = 0; i < len/16; i++) {
        rc6_enc(p, ctx);
        p += 16;
    }

    return p;
}

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
    struct imagewty_header *header;
    group_t *head, *filelist;
    uint32_t i, num_files;
    variable_t *var;
    FILE *cfp, *ofp;
    void *p;

    /* try to open image configuration */
    cfp = dir_fopen(indn, "image.cfg", "r");
    if (cfp == NULL) {
        fprintf(stderr, "Error: unable to open %s/%s!\n", indn, "image.cfg");
        return -ENOENT;
    }

    ofp = fopen(outfn, "wb");
    if (ofp == NULL) {
        fprintf(stderr, "Error: could not create image file %s!\n", outfn);
        fclose(cfp);
        return -ENOENT;
    }

    /* file is there, now try to load it */
    head = cfg_load(cfp);
    fclose(cfp);
    if (head == NULL) {
        fprintf(stderr, "Error: failed to parse %s/%s!\n", indn, "image.cfg");
        return -EINVAL;
    }

    /* Got configuration, time to start packing it all up! */
    var = cfg_find_var("filelist", head);
    if (var == NULL || var->type != VT_STRING) {
        fprintf(stderr, "Error: Unable to find filelist string "
                        "variable in configuration!\n");
        cfg_free(head);
        return -EINVAL;
    }

    filelist = cfg_find_group(var->str, head);
    if (filelist == NULL) {
        fprintf(stderr, "Error: unable to find group %s!\n", var->str);
        cfg_free(head);
        return -EINVAL;
    }

    num_files = cfg_count_vars(filelist);
    if (!num_files) {
        fprintf(stderr, "Error: no files to pack found in configuration!\n");
        cfg_free(head);
        return -EINVAL;
    }

    p = malloc(1024 + num_files * 1024);
    if (p == NULL) {
        fprintf(stderr, "Error: failed to allocate memory for image!\n");
        cfg_free(head);
        return -ENOMEM;
    }

    /* Initialize image file header */
    memset(p, 0, 1024 + num_files * 1024);
    header = p;
    memcpy(header->magic, IMAGEWTY_MAGIC, sizeof(header->magic));
    header->header_version = 0x0100;
    header->header_size = 0x50; /* should be 0x60 for version == 0x0300 */
    header->ram_base = 0x04D00000;
    header->version = cfg_get_number("version", head);
    header->image_size = 0; /* this will be filled in later on */
    header->image_header_size = 1024;
    header->v1.pid = cfg_get_number("pid", head);
    header->v1.vid = cfg_get_number("vid", head);
    header->v1.hardware_id = cfg_get_number("hardwareid", head);
    header->v1.firmware_id = cfg_get_number("firmwareid", head);
    header->v1.val1 = 1;
    header->v1.val1024 = 1024;
    header->v1.num_files = num_files;
    header->v1.num_files = cfg_count_vars(filelist);
    header->v1.val1024_2 = 1024;

    /* Setup file headers */
    {
        uint32_t offset = (num_files +1) * 1024;
        struct imagewty_file_header *fheaders;
        variable_t *var;

        fheaders = (struct imagewty_file_header*) (p + 1024);
        for(var=filelist->vars; var; var=var->next) {
            variable_t *v, *fn = NULL, *mt = NULL, *st = NULL;
            uint32_t size;
            FILE *fp;
            for (v=var->items; v; v=v->next) {
                if (v->type != VT_STRING)
                    continue;
                if (strcmp(v->name, "filename") == 0)
                    fn = v;
                else if (strcmp(v->name, "maintype") == 0)
                    mt = v;
                else if (strcmp(v->name, "subtype") == 0)
                    st = v;
            }

            if (!fn || !mt || !st) {
                fprintf(stderr, "Error: incomplete filelist item!\n");
                return -EINVAL;
            }

            fheaders->filename_len = IMAGEWTY_FHDR_FILENAME_LEN;
            fheaders->total_header_size = 1024;
            strcpy((char*)fheaders->v1.filename, fn->str);
            strcpy((char*)fheaders->maintype, mt->str);
            strcpy((char*)fheaders->subtype, st->str);

            fp = dir_fopen(indn, fn->str, "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                size = ftell(fp);
                fclose(fp);
            } else {
                fprintf(stderr, "Error: unable to read file '%s'!\n", fn->str);
                continue;
            }

            fheaders->v1.offset = offset;
            fheaders->v1.stored_length =
            fheaders->v1.original_length = size;
            if (fheaders->v1.stored_length & 0x1FF) {
                fheaders->v1.stored_length &= ~0x1FF;
                fheaders->v1.stored_length += 0x200;
            }
            offset += fheaders->v1.stored_length;
            fheaders = (struct imagewty_file_header*) ((uint8_t*)fheaders + 1024);
        }

        /* We now know the total size of the file; patch it into the main header */
        if (offset & 0xFF)
            offset = (offset & 0xFF) + 0x100;

        header->image_size = offset;
    }

    /* Now we have all headers setup in memory, time to write out the image file */
    fwrite(p, 1024, num_files +1, ofp);

    /* now write the file content too */
    for (i = 1; i <= num_files; i++) {
        struct imagewty_file_header *h =
            (struct imagewty_file_header*)(p + i * 1024);

        FILE *fp = dir_fopen(indn, h->v1.filename, "rb");
        if (fp != NULL) {
            char buf[512];
            size_t size = 0;
            while(!feof(fp)) {
                size_t bytesread = fread(buf, 1, 512, fp);
                if (bytesread) {
                    if (bytesread & 0x1ff)
                        bytesread = (bytesread & ~0x1ff) + 0x200;

                    if (flag_encryption_enabled)
                        rc6_encrypt_inplace(buf, bytesread, &filecontent_ctx);
                    fwrite(buf, 1, bytesread, ofp);
                }
                size += bytesread;
            }
            fclose(fp);
        }
    }

    /* Headers no longer used; encrypt and write if requested */
    if (flag_encryption_enabled) {
        void *curr = rc6_encrypt_inplace(p, 1024, &header_ctx);
        rc6_encrypt_inplace(curr, num_files * 1024, &fileheaders_ctx);
        rewind(ofp);
        fwrite(p, 1024, num_files +1, ofp);
    }

    fclose(ofp);

    /* Done, free configuration, no longer needed */
    cfg_free(head);

    return 0;
}

static int
decrypt_image(const char *infn, const char *outfn)
{
    int num_files, pid, vid, hardware_id, firmware_id;
    struct imagewty_header *header;
    void *image, *curr;
    FILE *ifp, *ofp;
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
        if (TF_DECRYPT_WORKING &&
            !(strlen(filename) >= 4 &&
            strncmp(filename + strlen(filename) -4, ".fex", 4) == 0)) {
            /* Not a 'FEX' file, so we need to decrypt it even more! */
            tf_decrypt_inplace(curr, stored_length);
        }
        curr = next;
    }

    ofp = fopen(outfn, "wb");
    if (ofp == NULL) {
        fprintf(stderr, "Error: unable to open output file %s!\n", outfn);
        return 4;
    }

    fwrite(image, imagesize, 1, ofp);
    fclose(ofp);

    return 0;
}

static int
unpack_image(const char *infn, const char *outdn)
{
    int num_files, pid, vid, hardware_id, firmware_id;
    FILE *ifp, *lfp = NULL, *ofp, *cfp;
    struct imagewty_header *header;
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
        if (TF_DECRYPT_WORKING &&
            !(strlen(filename) >= 4 &&
            strncmp(filename + strlen(filename) -4, ".fex", 4) == 0)) {
            /* Not a 'FEX' file, so we need to decrypt it even more! */
            tf_decrypt_inplace(curr, stored_length);
        }
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
    }
    cfp = dir_fopen(outdn, "image.cfg", "wb");
    if (cfp != NULL) {
        char timestr[256];
        struct tm *tm;
        time_t t;
        time(&t);
        tm = localtime(&t);
        strcpy(timestr, asctime(tm));
        /* strip newline */
        timestr[strlen(timestr) -1] = '\0';

        fputs(";/**************************************************************************/\r\n", cfp);
        fprintf(cfp, "; %s\r\n", timestr);
        fprintf(cfp, "; generated by %s\r\n", progname);
        fprintf(cfp, "; %s\r\n", infn);
        fputs(";/**************************************************************************/\r\n", cfp);
        fputs("[DIR_DEF]\r\n", cfp);
#ifdef WIN32
        fputs("INPUT_DIR = \".\\\\\"\r\n\r\n", cfp);
#else
        fputs("INPUT_DIR = \"./\"\r\n\r\n", cfp);
#endif
        fputs("[FILELIST]\r\n", cfp);
    }

    for (i=0; i < num_files; i++) {
        uint32_t stored_length, original_length;
        struct imagewty_file_header *filehdr;
        char hdrfname[32], contfname[32];
        const char *filename;
        uint32_t offset;

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
            printf("Extracting: %.8s %.16s (%u, %u)\n",
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

            fprintf(cfp, "\t{filename = INPUT_DIR .. \"%s\", maintype = \"%.8s\", subtype = \"%.16s\",},\r\n",
                contfname,
                filehdr->maintype, filehdr->subtype);
        } else if (flag_compat_output == OUTPUT_IMGREPACKER) {
	    printf("Extracting %s\n", filename);

            ofp = dir_fopen(outdn, filename, "wb");
            if (ofp) {
                fwrite(image + offset, original_length, 1, ofp);
                fclose(ofp);
            }

            fprintf(cfp, "\t{filename = INPUT_DIR .. \"%s\", maintype = \"%.8s\", subtype = \"%.16s\",},\r\n",
                filename[0] == '/' ? filename+1 : filename,
                filehdr->maintype, filehdr->subtype);
        }
    }

    if (cfp != NULL) {
        /* Now print the relevant stuff for the image.cfg */
        fputs("\r\n[IMAGE_CFG]\r\n", cfp);
        fprintf(cfp, "version = 0x%06x\r\n", header->version);
        fprintf(cfp, "pid = 0x%08x\r\n", pid);
        fprintf(cfp, "vid = 0x%08x\r\n", vid);
        fprintf(cfp, "hardwareid = 0x%03x\r\n", hardware_id);
        fprintf(cfp, "firmwareid = 0x%03x\r\n", firmware_id);
        fprintf(cfp, "imagename = \"%s\"\r\n", infn);
        fputs("filelist = FILELIST\r\n", cfp);
        fclose(cfp);
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
        c = getopt(argc, argv, "vdurhn");
        if (c == EOF)
            break;
        switch (c) {
	case 'n':
            flag_encryption_enabled = 0;
            break;
        case 'u':
            flag_compat_output = OUTPUT_UNIMG;
            break;
        case 'r':
            flag_compat_output = OUTPUT_IMGREPACKER;
            break;
        case 'd':
            flag_dump_decrypted = 1;
            break;
        case 'v':
            flag_verbose++;
            break;
        case 'h':
            fprintf(stderr, "%s [-vurh] {image dir|dir image}\n"
                    "  -r         imgRepacker compatibility\n"
                    "  -u         unimg.exe compatibility\n"
                    "  -n         disable encryption/decryption\n"
                    "  -d         dump decrypted image\n"
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
        if (flag_dump_decrypted) {
            /* Special case for only decrypting image */
            if (out == NULL) {
                strcpy(outfn, in);
                strcat(outfn, ".decrypted");
                out = outfn;
            }
            fprintf(stderr, "%s: decrypting %s into %s\n", argv[0], in, out);
            return decrypt_image(in, out);
        } else if (out == NULL) {
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
