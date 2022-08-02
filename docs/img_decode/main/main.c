/*
 * (C) Copyright 2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <ctype.h>
#include <sys/types.h>
#include <common.h>
#include "firmware/imgdecode.h"


#define  PARSER_BUFFER_MAX    (32 * 1024 * 1024)
static void *img_hd = NULL;

static int img_parser_file(void *img_hd, char *name, char *out_dir);
static int img_list_files(void *img_hd);
static int decode_all_item(void *img_hd, char *out_dir);

static void usage(void)
{
	printf("*************************start**************************\n");
	printf("*                                                      *\n");
	printf(" the usage of imgdecode:\n");
	printf(" when the program can work well, it will make a directory named imgdecode to store the files.\n");
	printf(" If the directory exists already, this step would be ignored\n");
	printf(" imgdecode imgname                 decode all files in the firmware\n");
	printf(" imadecode imgname -list           list all the mainkey and subkey in the img\n");
	printf(" imadecode imgname -name SUBKEY    decode the files whose sub  key = SUBKEY\n");
	printf("*                                                      *\n");
	printf("**************************end***************************\n");

	return;
}

int main(int argc, char* argv[])
{
	char *keyname = NULL;
	int  i, list_cmd = 0;
	char cmdline[1024];
	char imgfullname[MAX_PATH] = "";
	char tmpdirpath[MAX_PATH];
	FILE *img_file;
	int decode_all = 0;

	if (argc == 2) {
		printf("findout all files\n");
		decode_all = 1;
	} else if ((argc != 3) && (argc != 4)) {
		printf("Dragon Img Decode: not enough parameters\n");
		usage();

		return -1;
	}

	for (i = 2; i < argc; i += 2) {
		if (!strcmp(argv[i], "-name")) {
			keyname = argv[i + 1];
			printf("findout all files whose main name is %s\n", keyname);
		} else if (!strcmp(argv[i], "-list")) {
			i--;
			printf("try to list all the subkey in the image\n");
			printf("all the other command will be ingored\n");
			list_cmd = 1;
		} else {
			printf("Dragon Img Decode: Unknown command\n");

			usage();
			return -1;
		}
	}

	GetFullPath(imgfullname, argv[1]);
	printf("imgpath=%s\n", imgfullname);

	img_file = fopen(imgfullname, "rb");

	if (img_file == NULL) {
		printf("Dragon Img Decode: The file cant be open\n");
		usage();

		return -1;
	}

	fclose(img_file);

	img_hd = Img_Open(imgfullname);

	if (!img_hd) {
		printf("Dragon Img Decode: the iamge file is invalid\n");

		return -1;
	}

	if (list_cmd) {
		img_list_files(img_hd);

		return 0;
	}

	memset(tmpdirpath, 0, MAX_PATH);
	GetFullPath(tmpdirpath, "imgout");

	memset(cmdline, 0, 1024);
	sprintf(cmdline, "rm -rf %s", tmpdirpath);
	system(cmdline);

	memset(cmdline, 0, 1024);
	sprintf(cmdline, "mkdir -p %s", tmpdirpath);
	system(cmdline);

	if (decode_all) {
		decode_all_item(img_hd, tmpdirpath);
	} else {
		if (img_parser_file(img_hd, keyname, tmpdirpath)) {
			usage();

			return -1;
		}
	}

	return 0;
}

static int img_parser_file(void *img_hd, char *name, char *out_dir)
{
	char   outfullpath[MAX_PATH];
	FILE   *dedicate_file = NULL;
	long long file_len, tmp_file_len;
	char *buffer;
	uint   file_offset, read_len;
	void *item_hd;
	int   ret = -1;

	item_hd = Img_OpenItem(img_hd, name);

	if (!item_hd) {
		printf("Dragon Img Decode: the wanted file is not exist\n");

		return -1;
	}

	file_len = Img_GetItemSize(img_hd, item_hd);

	if (!file_len) {
		printf("Dragon Img Decode: the dedicate file length is 0\n");

		goto __parser_img_out;
	}

	memset(outfullpath, 0, MAX_PATH);
	sprintf(outfullpath, "%s/%s.bin", out_dir, name);

	dedicate_file = fopen(outfullpath, "wb");

	if (dedicate_file == NULL) {
		printf("Dragon Img Decode: unable to create the dedicate file\n");

		goto __parser_img_out;
	}

	buffer = (char *)malloc(PARSER_BUFFER_MAX);

	if (buffer == NULL) {
		printf("Dragon Img Decode: unable to malloc buffer to store data\n");

		goto __parser_img_out;
	}

	file_offset = 0;
	tmp_file_len = file_len;

	while (tmp_file_len >= PARSER_BUFFER_MAX) {
		read_len = Img_ReadItem_Continue(img_hd, item_hd, buffer, PARSER_BUFFER_MAX, file_offset);

		if (read_len != PARSER_BUFFER_MAX) {
			printf("Dragon Img Decode: read(step1) dedicate file err\n");

			goto __parser_img_out;
		}

		fwrite(buffer, PARSER_BUFFER_MAX, 1, dedicate_file);
		file_offset += PARSER_BUFFER_MAX;
		tmp_file_len -= PARSER_BUFFER_MAX;
	}

	if (tmp_file_len) {
		read_len = Img_ReadItem_Continue(img_hd, item_hd, buffer, (uint)tmp_file_len, file_offset);

		if (read_len != tmp_file_len) {
			printf("Dragon Img Decode: read(step2) dedicate file err\n");

			goto __parser_img_out;
		}

		fwrite(buffer, (uint)tmp_file_len, 1, dedicate_file);
	}

	printf("successfully writing the dedicate file %s\n", outfullpath);
	ret = 0;

__parser_img_out:

	if (dedicate_file) {
		fclose(dedicate_file);
	}

	if (buffer) {
		free(buffer);
	}

	if (item_hd) {
		Img_CloseItem(img_hd, item_hd);
	}

	return ret;
}

static int decode_all_item(void *image_name, char *out_dir)
{
	u8   *name;
	int   index;

	index = 0;

	printf("Ready to decode All Item Name\n\n");

	do {
		name = Img_GetItem_Subname(img_hd, index);

		if (name != NULL) {
			index ++;

			if (img_parser_file(image_name, name, out_dir)) {
				return -1;
			}
		} else {
			break;
		}
	} while (1);

	return 0;
}

static int img_list_files(void *img_hd)
{
	u8   *name;
	int   index;

	index = 0;

	printf("Ready to List All Item Name\n\n");

	do {
		name = Img_GetItem_Subname(img_hd, index);

		if (name != NULL) {
			index ++;
			printf("Item %4d: %s\n", index, name);
		} else {
			break;
		}
	} while (1);

	return 0;
}


