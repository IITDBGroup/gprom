ACLOCAL_AMFLAGS = -I m4

# add include dir to C compile flags
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -I$(top_srcdir)/include @GPROM_CFLAGS@

# subdirs to build
SUBDIRS = src test

# additional directories to include in a distribution package:
# - maintainer Makefile for running autotools 
# - the header file dir 
# - .y and .l files
# - YACC and FLEX generated files
# - scripts for running executables and scripts
EXTRA_DIST = $(top_srcdir)/Makefile.maintainer \
	$(top_srcdir)/include \
	$(top_srcdir)/src/parser/*.y \
	$(top_srcdir)/src/parser/*.l \
	$(top_srcdir)/src/parser/*.lex.c \
	$(top_srcdir)/src/parser/*.tab.c \
	$(top_srcdir)/src/parser/*.tab.h \
	$(top_srcdir)/test/testmain.sh