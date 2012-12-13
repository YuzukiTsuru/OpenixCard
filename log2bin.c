/*
 * log2bin.c, small filter to convert filtered UsbSnoop log data to binary file.
 * Copyright (c) 2012, Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and COPYING for more details.
 */

#include <stdio.h>

int main(int argc, char **argv)
{
	int buf[16];
	int off, cnt, i;

	while(!feof(stdin)) {
		cnt = fscanf(stdin, "    %08x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			&off,	
			buf, buf+1, buf+2, buf+3, buf+4, buf+5, buf+6, buf+7, buf+8, buf+9, buf+10, buf+11, buf+12, buf+13, buf+14, buf+15);
		if (cnt <= 1) break;

		/* do not count the offset specifier */
		cnt--;
		fprintf(stderr, "%s: processing offset %08x (l:%d)...", argv[0], off, cnt);

		/* output read buffer */
		for (i=0; i < cnt; i++)
			fputc(buf[i], stdout);

		fputc('\n', stderr);
	}

	return 0;
}
