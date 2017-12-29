AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

lib_LTLIBRARIES = libgprom.la

LIBTOOL_VERSION_STRING = $(subst .,:,$(VERSION))

all-local:
	echo $(LIBTOOL_VERSION_STRING) $(VERSION)

libgprom_la_SOURCES = libgprom.c
libgprom_la_LIBADD = ../libsrc.la
libgprom_la_LDFLAGS = -version-info $(LIBTOOL_VERSION_STRING) -no-undefined 
# libgprom_la_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/common
