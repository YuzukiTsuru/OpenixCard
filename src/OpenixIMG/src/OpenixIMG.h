//
// Created by gloom on 2022/2/13.
//

#ifndef OPENIXCARD_OPENIXIMG_H
#define OPENIXCARD_OPENIXIMG_H

#include "CFGParser.h"

#include "twofish.h"
#include "../lib/rc6/src/rc6.h"

#include "IMAGEWTY.h"

#define TF_DECRYPT_WORKING 0

#ifdef WIN32
#define MKDIR(p)    mkdir(p)
#else
#define MKDIR(p)    mkdir(p,S_IRWXU)
#endif

enum {
    OUTPUT_IMGREPACKER,
    OUTPUT_UNIMG,
};

int flag_encryption_enabled, flag_compat_output;

/* Crypto */
rc6_ctx_t header_ctx;
rc6_ctx_t fileheaders_ctx;
rc6_ctx_t filecontent_ctx;
u4byte tf_key[32];

const char *progname;

void recursive_mkdir(const char *dir);
void crypto_init(void);
void *rc6_decrypt_inplace(void *p, size_t len, rc6_ctx_t *ctx);
void *rc6_encrypt_inplace(void *p, size_t len, rc6_ctx_t *ctx);
void *tf_decrypt_inplace(void *p, size_t len);
FILE *dir_fopen(const char *dir, const char *path, const char *mode);
int unpack_image(const char *infn, const char *outdn);

#endif //OPENIXCARD_OPENIXIMG_H
