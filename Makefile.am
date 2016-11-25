ACLOCAL_AMFLAGS = -I m4

# add include dir to C compile flags
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -I$(top_srcdir)/include @GPROM_CFLAGS@

# subdirs to build
if JAVABINDING
SUBDIRS = include src test javatest doc
TESTS = blackboxtests/runtests.sh
else
SUBDIRS = include src test doc
endif

# additional directories to include in a distribution package:
# - maintainer Makefile for running autotools 
# - the header file dir 
# - .y and .l files
# - YACC and FLEX generated files
# - scripts for running executables and scripts
# - java source code and build files (ivy/ant)
EXTRA_DIST = $(top_srcdir)/Makefile.maintainer \
	$(top_builddir)/build.xml \
	$(top_builddir)/ivy.xml \
	$(top_builddir)/include \
	$(top_builddir)/src/interfaces/jdbc \
	$(top_builddir)/src/parser/*.y \
	$(top_builddir)/src/parser/*.l \
	$(top_builddir)/src/parser/*.lex.c \
	$(top_builddir)/src/parser/*.tab.c \
	$(top_builddir)/src/parser/*.tab.h \
	$(top_builddir)/scripts/*.sh \
	$(top_builddir)/scripts/dev/*.sh \
	$(top_builddir)/examples \
	$(top_builddir)/doc/gprom.man \
	$(top_builddir)/javatest \
	$(top_builddir)/blackboxtests/build.xml \
	$(top_builddir)/blackboxtests/PI_CS_queries/*.xml \
	$(top_builddir)/blackboxtests/testdb/testDB.sql.in \
	$(top_builddir)/blackboxtests/TestTemplates/*.stg  
	
	

# TODO add java
