ACLOCAL_AMFLAGS = -I m4

# add include dir to C compile flags
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -I$(top_srcdir)/include

# subdirs to build
SUBDIRS = src test

# additional directories to include in a distribution packing include the header file dir and parser
EXTRA_DIST = include $(top_srcdir)/src/parser/sql_parser.l $(top_srcdir)/src/parser/sql_parser.y \
	$(top_srcdir)/src/parser/sql_parser.tab.h
