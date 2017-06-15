AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

lib_LTLIBRARIES = libgprom.la	libgprom_internal.la

libgprom_la_SOURCES = libgprom.c
libgprom_la_LIBADD = ../libsrc.la

libgprom_internal_la_SOURCES = libgprom_internal.c
libgprom_internal_la_LIBADD = ../libsrc.la

