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
    int rc;
    if (argc - optind > 2) {
        fprintf(stderr, "%s: extra arguments\n", argv[0]);
        return 1;
    } else if (argc - optind < 1) {
        fprintf(stderr, "%s: missing file argument\n", argv[0]);
        return 1;
    }
    /* If we get here, we have a file spec and possibly options */
    //crypto_init();
    rc = stat(argv[optind], &statbuf);
    if (rc) {
        fprintf(stderr, "%s: cannot stat '%s'!\n", argv[0], argv[optind]);
        return 1;
    }
    out = (argc - optind) == 2 ? argv[optind + 1] : NULL;
    in = argv[optind];
    if (out == NULL) {
        strcpy(outfn, in);
        strcat(outfn, ".dump");
    } else {
        strcpy(outfn, out);
    }
    out = outfn;
    unpack_image(in, out);
    return 0;
}