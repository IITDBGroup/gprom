AM_CPPFLAGS = -I$(top_srcdir)/include

SUBDIRS = pi_cs_rewrites transformation_rewrites

noinst_LTLIBRARIES        				= libprovenance_rewriter.la
libprovenance_rewriter_la_SOURCES		= prov_rewriter_main.c prov_utility.c prov_schema.c
libprovenance_rewriter_la_LIBADD      	= pi_cs_rewrites/libpi_cs_rewrites.la \
										transformation_rewrites/libtransformation_rewrites.la