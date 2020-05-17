AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

SUBDIRS = pi_cs_rewrites transformation_rewrites update_and_transaction game_provenance xml_rewrites summarization_rewrites semiring_combiner uncertainty_rewrites lateral_rewrites coarse_grained unnest_rewrites update_ps

noinst_LTLIBRARIES        				= libprovenance_rewriter.la
libprovenance_rewriter_la_SOURCES		= prov_rewriter_main.c prov_utility.c prov_schema.c
libprovenance_rewriter_la_LIBADD      	= pi_cs_rewrites/libpi_cs_rewrites.la \
										transformation_rewrites/libtransformation_rewrites.la \
										update_and_transaction/libupdate_rewrites.la \
										game_provenance/libgp.la \
										xml_rewrites/libxml_rewrites.la \
										lateral_rewrites/liblateral_rewrites.la \
										unnest_rewrites/libunnest_rewrites.la \
										summarization_rewrites/libsummarization_rewrites.la \
										semiring_combiner/libsc.la \
										uncertainty_rewrites/libur.la \
										coarse_grained/libcoarse_grained_rewrites.la \
										update_ps/libupdate_ps.la
										
