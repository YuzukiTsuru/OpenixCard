//
// Created by gloom on 2022/2/13.
//
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "OpenixIMG.h"

int main(int argc, char **argv) {
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
    flag_compat_output = OUTPUT_IMGREPACKER;

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

    out = (argc - optind) == 2 ? argv[optind + 1] : NULL;
    in = argv[optind];

    if (S_ISDIR(statbuf.st_mode)) {
        /* We're packing, lets see if we have to generate the output image filename ourselfs */
        if (out == NULL) {
            int len = strlen(in);
            strcpy(outfn, in);
            if (in[len - 1] == '/') len--;
            if (len > 5 && strncmp(in + len - 5, ".dump", 5) == 0)
                outfn[len - 5] = '\0';
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