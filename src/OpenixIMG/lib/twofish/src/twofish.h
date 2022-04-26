#ifndef TWOFISH_H
#define TWOFISH_H

typedef unsigned long u4byte; /* a 32 bit unsigned integer type   */

void tf_encrypt(const u4byte in_blk[4], u4byte out_blk[]);

void tf_decrypt(const u4byte in_blk[4], u4byte out_blk[4]);

u4byte *tf_init(const u4byte in_key[], u4byte key_len);

#endif
