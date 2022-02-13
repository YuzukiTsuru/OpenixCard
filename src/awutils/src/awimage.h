//
// Created by gloom on 2022/2/13.
//

#ifndef OPENIXCARD_AWIMAGE_H
#define OPENIXCARD_AWIMAGE_H

#include "parsecfg.h"

#include "twofish.h"
#include "rc6.h"

#include "imagewty.h"

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

int flag_encryption_enabled, flag_compat_output, flag_dump_decrypted, flag_verbose;

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
int pack_image(const char *indn, const char *outfn);
int decrypt_image(const char *infn, const char *outfn);
int unpack_image(const char *infn, const char *outdn);

#endif //OPENIXCARD_AWIMAGE_H
