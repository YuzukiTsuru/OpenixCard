if BUILD_SILENTLY
AM_MAKEFLAGS = --no-print-directory
endif

EXTRA_DIST = \
	README.rst \
	test.config \
	flash.conf

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h

noinst_LIBRARIES = libgenimage.a
libgenimage_a_SOURCES = \
	genimage.c \
	config.c \
	util.c \
	crc32.c \
	image-android-sparse.c \
	image-cpio.c \
	image-cramfs.c \
	image-ext2.c \
	image-file.c \
	image-fit.c \
	image-flash.c \
	image-hd.c \
	image-iso.c \
	image-jffs2.c \
	image-qemu.c \
	image-rauc.c \
	image-squashfs.c \
	image-tar.c \
	image-ubi.c \
	image-ubifs.c \
	image-vfat.c

libgenimage_a_CFLAGS = \
	$(AM_CFLAGS) \
	$(CONFUSE_CFLAGS)

libgenimage_a_LIBADD = \
	$(CONFUSE_LIBS)

noinst_HEADERS = \
	genimage.h \
	list.h

# when "make distclean" runs
DISTCLEAN = \
	configure \
	autoscan.log \
	config.h.in~ \
	config.h.in \
	configure.scan \
	configure.ac~ \
	aclocal.m4 \
	Makefile.in \
	build-aux/depcomp \
	build-aux/install-sh \
	build-aux/missing \
	Makefile \
	.deps \
	autom4te.cache \
	$(DIST_ARCHIVES)
