AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

SUBDIRS = pi_cs_rewrites transformation_rewrites update_and_transaction game_provenance

noinst_LTLIBRARIES        				= libprovenance_rewriter.la
libprovenance_rewriter_la_SOURCES		= prov_rewriter_main.c prov_utility.c prov_schema.c
libprovenance_rewriter_la_LIBADD      	= pi_cs_rewrites/libpi_cs_rewrites.la \
										transformation_rewrites/libtransformation_rewrites.la \
										update_and_transaction/libupdate_rewrites.la \
										game_provenance/libgp.la