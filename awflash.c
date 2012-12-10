/*
 * awflash.c, flash Allwinner A10 devices with Livesuit USB protocol.
 * Copyright (c) 2012, Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and COPYING for more details.
 */

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <usb.h>

#define VENDORID_ALLWINNER	0x1f3a
#define PRODUCTID_SUN4I		0xefe8

#define _BLOCKSIZE		(64UL * 1024UL)

static char *progname;
static int verbose = 0;

static const char *sTabTab = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
#define DS "%.*s"
#define DA depth - 1, sTabTab

static void dump_hex(const uint32_t *data, int len, int depth)
{
	char str[128];
	char astr[32];
	char *p;
	int l;
	int i;

	for (i = 0; i < len; ) {
		p = str;
		l = sizeof(str);
		for (; i < len && (p == str || (i % 16 != 0)); i++) {
			snprintf(p, l - 1, "%02x ", data[i]);
			l -= strlen(p);
			p += strlen(p);
			astr[i % 16] = isprint(data[i]) ? data[i] : '.';
			astr[i % 16] = isprint(data[i]) ? data[i] : '.';
			astr[(i % 16) + 1] = '\0';
		}
		printf(DS"    %-48.48s	%s\n", DA, str, astr);
	}
}


static int
match_device(int vendorid, int productid)
{
	return vendorid == VENDORID_ALLWINNER && productid == PRODUCTID_SUN4I;
}

static usb_dev_handle *
locate_device(void)
{
	unsigned char located = 0;
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *device_handle = 0;

	usb_find_busses();
	usb_find_devices();

	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (match_device(dev->descriptor.idVendor, dev->descriptor.idProduct)) {
				located++;
				device_handle = usb_open(dev);
				if (verbose)
					printf("Device Found @ Address %s\n", dev->filename);
			}
		}
	}

	return device_handle;
}

static void
usage(void)
{

	fprintf(stderr, "usage: %s [-B] [-r size] offset file\n", progname);
	exit(EXIT_FAILURE);
}

#define FEX_TIMEOUT	500

#define FEX_EP_OUT	1
#define FEX_EP_IN	2

#define FEX_DIR_IN	0x11
#define FEX_DIR_OUT	0x12

#define FEX_CMAGIC	"AWUC"
#define FEX_SMAGIC	"AWUS"

#define FEX_ACK_LEN	13

static void
fex_xfer(struct usb_dev_handle *hdl, void *buffer, size_t len, uint8_t dir)
{
	char ackbuf[FEX_ACK_LEN];
	int status;

	struct cmd {
		char magic[4];
		uint32_t pad0;
		uint32_t len1;
		uint8_t pad1[3];
		uint8_t val_0c;
		uint8_t dir;
		uint8_t pad2;
		uint32_t len2;
		uint8_t pad3[10];
	} __attribute__((packed)) cmd;

	memset(&cmd, 0, sizeof(cmd));
	memcpy(cmd.magic, FEX_CMAGIC, 4);
	cmd.len1 = cmd.len2 = len;
	cmd.val_0c = 0x0c;
	cmd.dir = dir;

	status = usb_bulk_write(hdl, FEX_EP_OUT, (void*)&cmd, sizeof(cmd), FEX_TIMEOUT);
	if (status != sizeof(cmd)) {
		if (verbose) printf("%s: cmd write failed (%d/%lu)!\n", __func__, status, sizeof(cmd));
		return (status < 0) ? status : -EIO;
	}

	if (dir == FEX_DIR_IN) {
		status = usb_bulk_read(hdl, FEX_EP_IN, buffer, len, FEX_TIMEOUT);
		if (status != len) {
			if (verbose) printf("%s: data read failed (%d/%lu)!\n", __func__, status, len);
			return (status < 0) ? status : -EIO;
		}
	} else if (dir == FEX_DIR_OUT) {
		status = usb_bulk_write(hdl, FEX_EP_OUT, buffer, len, FEX_TIMEOUT);
		if (status != len) {
			if (verbose) printf("%s: data write failed (%d/%lu)!\n", __func__, status, len);
			return (status < 0) ? status : -EIO;
		}
	}

	status = usb_bulk_read(hdl, FEX_EP_IN, ackbuf, FEX_ACK_LEN, FEX_TIMEOUT);
	if (status != FEX_ACK_LEN) {
		if (verbose) printf("%s: status read failed (%d/%d)!\n", __func__, status, FEX_ACK_LEN);
		return (status < 0) ? status : -EIO;
	}

	if (memcmp(ackbuf, FEX_SMAGIC, 4) != 0) {
		if (verbose) printf("%s: Invalid response magic!\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static void
fex_command(struct usb_dev_handle *hdl, void *buffer1, void *buffer2, size_t len)
{
	int status = fex_xfer(hdl, buffer1, 16, FEX_DIR_OUT);
	if (status < 0)
		return status;

	uint16_t cmd = *(uint16_t*)buffer1;
	switch(cmd) {
		case 0x0001:
			if (buffer2 != NULL) {
				status = fex_xfer(hdl, buffer2, len, FEX_DIR_IN);
				if (status < 0)
					return status;

				if (strncmp(buffer2, "AWUSBFEX", 8) != 0) {
					if (verbose) printf("%s: non-AWUSBFEX response!\n", __func__);
					return -EINVAL;
				}
			} else {
				if (verbose) printf("%s: No buffer passed for cmd 0x0001!\n", __func__);
				return -EINVAL;
			}
			break;
		case 0x0002:
			break;
		case 0x0003:
			if (buffer2 != NULL) {
				status = fex_xfer(hdl, buffer2, len, FEX_DIR_IN);
				if (status < 0)
					return status;
			} else {
				if (verbose) printf("%s: No buffer passed for cmd 0x0003!\n", __func__);
				return -EINVAL;
			}
			break;
		case 0x0004:
			if (buffer2 != NULL) {
				status = fex_xfer(hdl, buffer2, len, FEX_DIR_IN);
				if (status < 0)
					return status;
			} else {
				if (verbose) printf("%s: No buffer passed for command 0x0004!\n", __func__);
				return -EINVAL;
			}
			break;
		case 0x0010:
			break;
		case 0x0101:
			if (buffer2 != NULL && len != 0) {
				status = fex_xfer(hdl, buffer2, len, FEX_DIR_OUT);
				if (status < 0)
					return status;
			}
			break;
		case 0x0102:
			break;
		case 0x0103:
			if (buffer2 != NULL) {
				status = fex_xfer(hdl, buffer2, len, FEX_DIR_IN);
				if (status < 0)
					return status;
			}
			break;
		case 0x0201:
			if (buffer2 != NULL && len != 0) {
				switch((((uint8_t*)buffer1)[13] >> 4) & 3) {
					case 0:
					case 1:
						status = fex_xfer(hdl, buffer2, len, FEX_DIR_OUT);
						if (status < 0)
							return status;
						break;
					case 2:
						status = fex_xfer(hdl, buffer2, len, FEX_DIR_IN);
						if (status < 0)
							return status;
						break;
					case 3:
						if (verbose) printf("%s: Invalid subcommand 3 in command 0x0201!\n", __func__);
						return -EINVAL;
				}
			}
			break;
		case 0x0202:
			if (buffer2 != NULL) {
				status = fex_xfer(hdl, buffer2, len, FEX_DIR_OUT);
				if (status < 0)
					return status;
			} else {
				if (verbose) printf("%s: No buffer passed for command 0x0202!\n", __func__);
				return -EINVAL;
			}
			break;
		case 0x0203:
			{
				uint8_t subcmd = ((uint8_t*)buffer1)[4];
				if (subcmd & 0xF0) {
					if ((subcmd & 0xF0) != 0x10) {
						if (verbose) printf("%s: Unrecognized subcommand 0x%x for command 0x0203!\n",
							__func__, subcmd);
						return -EINVAL;
					} else {
						status = fex_xfer(hdl, buffer2, len, (subcmd & 8) ? FEX_DIR_OUT : FEX_DIR_IN);
						if (status < 0)
							return status;
					}
				} else {
					if (buffer2 != NULL) {
						status = fex_xfer(hdl, buffer2, len, FEX_DIR_IN);
						if (status < 0)
							return status;
					} else {
						if (verbose) printf("%s: No buffer specified for command 0x0203!\n", __func__);
						return -EINVAL;
					}
				}
			}
			break;
		default:
			if (verbose) printf("%s: Unknown command 0x%04x passed!\n", __func__, cmd);
			return -EINVAL;
	}

	uint8_t statusbuf[8];
	status = fex_xfer(hdl, statusbuf, 8, FEX_DIR_IN);
	if (status < 0)
		return status;
	if (statusbuf[4] != 0) {
		printf("%s: Invalid status %d!\n", __func__, statusbuf[4]);
		return -EINVAL;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	struct usb_dev_handle *xsv_handle;
	int open_status;
	uint32_t off;
	int32_t size;
	uint8_t *buf;
	int ch, img, boot;

	progname = argv[0];

	boot = 0;
	size = 0;
	while ((ch = getopt(argc, argv, "vBr:")) != -1) {
		switch (ch) {
		case 'v':
			verbose++;
			break;
		case 'B':
			boot = 1;
			break;
		case 'r':
			size = strtoul(optarg, NULL, 0);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	if ((off = strtoul(argv[0], NULL, 0)) & 0x1f)
		errx(EXIT_FAILURE, "offset must be multiple of 0x20\n");

	if (size == 0)
		img = open(argv[1], O_RDONLY, 0);
	else
		img = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (img == -1)
		err(EXIT_FAILURE, "%s", argv[1]);

	if ((buf = malloc(_BLOCKSIZE)) == NULL)
		err(EXIT_FAILURE, "malloc");

	memset(buf, 0, _BLOCKSIZE);

	usb_init();
	if (verbose)
		usb_set_debug(verbose);

	if ((xsv_handle = locate_device()) == 0) {
		err(EXIT_FAILURE, "Could not find device\n");
		return (-1);
	}

	open_status = usb_set_configuration(xsv_handle,1);
	if (verbose)
		printf("conf_stat=%d\n",open_status);

	open_status = usb_claim_interface(xsv_handle,0);
	if (verbose)
		printf("claim_stat=%d\n",open_status);

	open_status = usb_set_altinterface(xsv_handle,0);
	if (verbose)
		printf("alt_stat=%d\n",open_status);

	char in2[0x08];
#if SIMULATE_UPGRADE_START
	char out1[] = {
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};
	char in1[0x20];

	/* URB 5/6/7 */
	fex_xfer(xsv_handle, out1, sizeof(out1), FEX_DIR_OUT);
	/* URB 8/9/10 */
	fex_xfer(xsv_handle, in1, sizeof(in1), FEX_DIR_IN);
	/* URB 11/12/13 */
	fex_xfer(xsv_handle, in2, sizeof(in2), FEX_DIR_IN);
#else /* Flash recovery */

	struct {
		uint32_t cmd;
		uint32_t arg1;
		uint32_t arg2;
		uint32_t arg3;
	} __attribute__((packed)) pos = {
		.cmd = 0x0201,
		.arg1 = off,
		.arg2 = _BLOCKSIZE,
		.arg3 = 0x5020,
	};

	do {
		int buflen = read(img, buf, _BLOCKSIZE);
		if (buflen <= 0)
			break;

		pos.arg2 = buflen;

		/* URB 32063/32064/32065 */
		fex_xfer(xsv_handle, &pos, sizeof(pos), FEX_DIR_OUT);

		/* URB 32066/32067/32068 */
		fex_xfer(xsv_handle, buf, buflen, FEX_DIR_OUT);

		/* URB 32069/32070/32071 */
		fex_xfer(xsv_handle, in2, sizeof(in2), FEX_DIR_IN);

		pos.arg1 += (buflen / 512);
		pos.arg3 = 0x1020;
	} while( 1 );
#endif
	close(img);

	return 0;
}

