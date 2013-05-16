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
#include <errno.h>

#include <usb.h>

#define VENDORID_ALLWINNER	0x1f3a
#define PRODUCTID_SUN4I		0xefe8

#define _BLOCKSIZE		(64UL * 1024UL)

static char *progname;
static int verbose = 0;

#ifdef DEBUG
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
#endif /* DEBUG */

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

static int
fex_xfer(libusb_device_handle *hdl, void *buffer, size_t len, uint8_t dir)
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

#define AW_FEL_VERSION	0x0001
#define AW_FEL_1_WRITE	0x0101
#define AW_FEL_1_EXEC	0x0102
#define AW_FEL_1_READ	0x0103

static int
fex_command(libusb_device_handle *hdl, uint32_t cmd, uint32_t arg1, uint32_t arg2, uint32_t arg3, void *data, size_t len)
{
	uint32_t cmdbuffer[4] = { cmd, arg1, arg2, arg3 };
	int status = fex_xfer(hdl, cmdbuffer, 16, FEX_DIR_OUT);
	if (status < 0)
		return status;

	switch(cmd) {
		case AW_FEL_VERSION:
			if (data != NULL) {
				status = fex_xfer(hdl, data, len, FEX_DIR_IN);
				if (status < 0)
					return status;

				if (strncmp(data, "AWUSBFEX", 8) != 0) {
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
			if (data != NULL) {
				status = fex_xfer(hdl, data, len, FEX_DIR_IN);
				if (status < 0)
					return status;
			} else {
				if (verbose) printf("%s: No buffer passed for cmd 0x0003!\n", __func__);
				return -EINVAL;
			}
			break;
		case 0x0004:
			if (data != NULL) {
				status = fex_xfer(hdl, data, len, FEX_DIR_IN);
				if (status < 0)
					return status;
			} else {
				if (verbose) printf("%s: No buffer passed for command 0x0004!\n", __func__);
				return -EINVAL;
			}
			break;
		case 0x0010:
			break;
		case AW_FEL_1_WRITE:
			if (data != NULL && len != 0) {
				status = fex_xfer(hdl, data, len, FEX_DIR_OUT);
				if (status < 0)
					return status;
			}
			break;
		case AW_FEL_1_EXEC:
			break;
		case AW_FEL_1_READ:
			if (data != NULL) {
				status = fex_xfer(hdl, data, len, FEX_DIR_IN);
				if (status < 0)
					return status;
			}
			break;
		case 0x0201:
			if (data != NULL && len != 0) {
				switch((((uint8_t*)cmdbuffer)[13] >> 4) & 3) {
					case 0:
					case 1:
						status = fex_xfer(hdl, data, len, FEX_DIR_OUT);
						if (status < 0)
							return status;
						break;
					case 2:
						status = fex_xfer(hdl, data, len, FEX_DIR_IN);
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
			if (data != NULL) {
				status = fex_xfer(hdl, data, len, FEX_DIR_OUT);
				if (status < 0)
					return status;
			} else {
				if (verbose) printf("%s: No buffer passed for command 0x0202!\n", __func__);
				return -EINVAL;
			}
			break;
		case 0x0203:
			{
				uint8_t subcmd = ((uint8_t*)cmdbuffer)[4];
				if (subcmd & 0xF0) {
					if ((subcmd & 0xF0) != 0x10) {
						if (verbose) printf("%s: Unrecognized subcommand 0x%x for command 0x0203!\n",
							__func__, subcmd);
						return -EINVAL;
					} else {
						status = fex_xfer(hdl, data, len, (subcmd & 8) ? FEX_DIR_OUT : FEX_DIR_IN);
						if (status < 0)
							return status;
					}
				} else {
					if (data != NULL) {
						status = fex_xfer(hdl, data, len, FEX_DIR_IN);
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

/* Return protocol version; or negative error code */
static int
aw_fel_get_version(libusb_device_handle *usb) {
        struct aw_fel_version {
                char signature[8];
                uint32_t unknown_08;    /* 0x00162300 */
                uint32_t unknown_0a;    /* 1 */
                uint16_t protocol;      /* 1 */
                uint8_t  unknown_12;    /* 0x44 */
                uint8_t  unknown_13;    /* 0x08 */
                uint32_t scratchpad;    /* 0x7e00 */
                uint32_t pad[2];        /* unused */
        } __attribute__((packed)) buf;
	int status;

	status = fex_command(usb, AW_FEL_VERSION, 0, 0, 0, buf, sizeof(buf));
	if (status < 0)
		return status;

	return buf.protocol;
}

int
main(int argc, char *argv[])
{
	libusb_device_handle *usb;

	int open_status;
	uint8_t *buf;
	int ch, img;

	progname = argv[0];

	while ((ch = getopt(argc, argv, "v")) != -1) {
		switch (ch) {
		case 'v':
			verbose++;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	img = open(argv[1], O_RDONLY, 0);
	if (img == -1)
		err(EXIT_FAILURE, "%s", argv[1]);

	if ((buf = malloc(_BLOCKSIZE)) == NULL)
		err(EXIT_FAILURE, "malloc");

	memset(buf, 0, _BLOCKSIZE);

	usb_init();
	if (verbose)
		usb_set_debug(verbose);

	if ((usb = locate_device()) == 0) {
		err(EXIT_FAILURE, "Could not find device\n");
		return (-1);
	}

	open_status = usb_set_configuration(usb,1);
	if (verbose)
		printf("conf_stat=%d\n",open_status);

	open_status = usb_claim_interface(usb,0);
	if (verbose)
		printf("claim_stat=%d\n",open_status);

	open_status = usb_set_altinterface(usb,0);
	if (verbose)
		printf("alt_stat=%d\n",open_status);

	/* Check device mode (FEX protocol version) */
	if (aw_get_version(usb) == 1) {
		/* We're version 1, so lets load the v2 protocol */
		char buffer[0x100];
		aw_get_version(usb);
		if (fex_command(usb, AW_FEL_1_READ, 0x7e00, sizeof(buffer), buffer, sizeof(buffer)) < 0 ||
		    *(uint32_t*)buffer == 0) {
			fprintf(stderr, "Error: device previously failed to switch; please power off/on device\n");
			exit(EXIT_FAILURE);
		}
		aw_get_version(usb);
		*(uint32_t*)buffer = 0;
		if (fex_command(usb, AW_FEL_1_WRITE, 0x7e00, sizeof(buffer), buffer, sizeof(buffer)) < 0) {
			fprintf(stderr, "Error: unable to write command to prepare to switch\n");
			exit(EXIT_FAILURE);
		}

		fex_command(usb, AW_FEL_1_WRITE, 0x7010, 180, buffer, 180);

		memset(buffer, 0, 16);
		fex_command(usb, AW_FEL_1_WRITE, 0x7210, 16, buffer, 16);

		fex_write_file(0x7220, "fex_1_1.fex", 0xae0);
		fex_command(usb, AW_FEL_1_EXEC, 0x7220, 0, NULL, 0);

		fex_command(usb, AW_FEL_1_READ, 0x7210, 16, buffer, 16);
		if (memcmp(buffer, "DRAM\x01", 5) != 0) {
			fprintf(stderr, "Error: could not find DRAM magic!\n");
			exit(EXIT_FAILURE);
		}

		fex_write_file(0x2000, "fes_1-2.fex", 0);
		fex_command(usb, AWL_FEL_1_EXEC, 0x2000, 0, NULL, 0);
	}

	close(img);

	return 0;
}

/*
 0x00002000 0x00000c1c fes_1-2.fex load area
 0x00007010 0x000000b4 see sys_para.txt
 0x00007210 0x00000010 "DRAM" magic
 0x00007220 0x00000ae0 fex_1_1.fex load area
 0x00007e00 0x00000100 marker block containing 0xcc values (first 4 bytes 0 after successful switch)
 0x00023000 0x00010000
 0x40100000 0x00002000
 0x40200000 0x00010000 fes.fex load area???
 0x40210000 0x00003c40 all 0x05 bytes
*/

/*
 A10 flash sequence:

 AW_FEL_VERSION
 AW_FEL_VERSION
 AW_FEL_1_READ  0x00007e00 0x100 (reads back block of 0xcc)
 AW_FEL_VERSION
 AW_FEL_1_WRITE 0x00007e00 0x100 (writes block of 0xcc; first 4 bytes 0x00)
 AW_FEL_1_WRITE 0x00007010 0xb4 (board & dram config)
 AW_FEL_1_WRITE 0x00007210 0x10 (all zeroes)
 AW_FEL_1_WRITE 0x00007220 0xae0 (fex_1_1.fex padded with zeroes at end)
 AW_FEL_1_EXEC  0x00007220
 AW_FEL_1_READ  0x00007210 0x10 (reads back "DRAM" padded with zeroes at end)
 AW_FEL_1_WRITE 0x00007210 0x10 (all zeroes)
 AW_FEL_1_WRITE 0x00002000 0xc1c (fes_1-2.fex)
 AW_FEL_1_EXEC  0x00002000
 AW_FEL_1_READ  0x00007210 0x10 ("DRAM\x01" padded with zeroes at end)
 AW_FEL_1_READ  0x00007010 0xb4 ()
 AW_FEL_1_WRITE 0x40100000 0x2000 ()
 AW_FEL_1_READ  0x40100000 0x2000 ()
 AW_FEL_1_WRITE 0x00007210 0x10 (all zeroes)
 AW_FEL_1_WRITE 0x00023000 0x10000 (all zeroes)
 AW_FEL_1_WRITE 0x40200000 0x10000 (fes.fex?)
 AW_FEL_1_WRITE 0x00023000 0x10000 (all zeroes)
 AW_FEL_1_WRITE 0x00033000 0x3c40 (all zeroes)
 AW_FEL_1_WRITE 0x40210000 0x3c40 (all 0x05 bytes)
 AW_FEL_1_WRITE 0x00033000 0x3c40 (all zeroes)
 AW_FEL_1_WRITE 0x00023000 0x5d0 (all zeroes)
 AW_FEL_1_WRITE 0x00007220 0x5d0 (fes_2.fex)
 AW_FEL_1_WRITE 0x00023000 0x5d0 (all zeroes)
 AW_FEL_1_EXEC  0x00007220
 <device reset>
 AW_FEL_VERSION
 AW_FEL_VERSION

*/
