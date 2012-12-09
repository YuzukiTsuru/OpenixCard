#ifndef TWOFISH_H
#define TWOFISH_H

#include "std_defs.h"

void tf_encrypt(const u4byte in_blk[4], u4byte out_blk[]);
void tf_decrypt(const u4byte in_blk[4], u4byte out_blk[4]);
u4byte *tf_init(const u4byte in_key[], const u4byte key_len);

#endif
