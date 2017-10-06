ACLOCAL_AMFLAGS = -I m4

# configure for distcheck (need to disable Java since Java build requires writing files and distcheck runs everything read-only 
AM_DISTCHECK_CONFIGURE_FLAGS=--disable-oracle --disable-java

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
	$(top_srcdir)/ant/one-jar* \
	$(top_srcdir)/ant/ivy.jar \
	$(top_srcdir)/build.xml \
	$(top_srcdir)/ivy.xml \
	$(top_builddir)/include \
	$(top_builddir)/src/interfaces/jdbc \
	$(top_srcdir)/src/parser/*.y \
	$(top_srcdir)/src/parser/*.l \
	$(top_srcdir)/src/parser/*.lex.c \
	$(top_srcdir)/src/parser/*.tab.c \
	$(top_srcdir)/src/parser/*.tab.h \
	$(top_srcdir)/scripts/*.sh \
	$(top_srcdir)/scripts/dev/*.sh \
	$(top_srcdir)/examples \
	$(top_builddir)/doc/gprom.man \
	$(top_builddir)/doc/gprom_man.html \
	$(top_builddir)/doc/gprom_man.md \
	$(top_builddir)/javatest \
	$(top_srcdir)/blackboxtests/build.xml \
	$(top_srcdir)/blackboxtests/testcases/*.xml \
	$(top_srcdir)/blackboxtests/testdb/testDB.sql.in \
	$(top_srcdir)/blackboxtests/TestTemplates/*.stg

gen-libgprom-header:
	javac -cp javalibgen/jnaerator.jar javalibgen/GenLibGProMH.java
	java -cp javalibgen/jnaerator.jar:javalibgen/ GenLibGProMH $(top_srcdir)/

# Debian packaging
APP_NAME=gprom
TMP_DIR=dpkg
TAR_PACKAGE=$(APP_NAME)-$(VERSION).tar.gz

# Create a deb package
pkg-deb: $(TAR_PACKAGE)
	rm -rf $(TMP_DIR)
	mkdir -p $(TMP_DIR)
	cp $(TAR_PACKAGE) $(TMP_DIR)/
	tar --directory $(TMP_DIR)/ \
		-xzf $(TMP_DIR)/$(TAR_PACKAGE)
	cd $(TMP_DIR)/$(APP_NAME)-$(VERSION)/; \
		echo | dh_make --single --copyright gpl -e bglavic@iit.edu -f \
			../$(TAR_PACKAGE)
	cp packaging/pkgfiles/changelog packaging/pkgfiles/control \
		packaging/pkgfiles/copyright $(TMP_DIR)/$(APP_NAME)-$(VERSION)/debian/
	cd $(TMP_DIR)/$(APP_NAME)-$(VERSION)/; \
		rm debian/README.Debian debian/*.ex debian/*.EX; \
		./configure --disable-oracle --disable-postgres --disable-java; \
		dpkg-buildpackage -rfakeroot; \
		mv ../*.deb $(PWD)/packaging
	rm -r $(TMP_DIR);
