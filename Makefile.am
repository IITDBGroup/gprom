ACLOCAL_AMFLAGS = -I m4

# add include dir to C compile flags
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -I$(top_srcdir)/include

# subdirs to build
SUBDIRS = src test

# additional directories to include in a distribution pacakge: 
# - the header file dir 
# - YACC and FLEX generated files
EXTRA_DIST = include \
	$(top_srcdir)/src/parser/sql_parser.lex.c \
	$(top_srcdir)/src/parser/sql_parser.tab.c \
	$(top_srcdir)/src/parser/sql_parser.tab.h
