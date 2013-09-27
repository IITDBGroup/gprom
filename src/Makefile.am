AM_CPPFLAGS = -I$(top_srcdir)/include

SUBDIRS = configuration log mem_manager model provenance_rewriter parser . command_line

noinst_LTLIBRARIES 		= libsrc.la
libsrc_la_LIBADD 		= 	configuration/libconfiguration.la \
						log/liblog.la \
						mem_manager/libmem_manager.la \
						model/libmodel.la \
						provenance_rewriter/libprovenance_rewriter.la \
						parser/libparser.la 

libsrc_la_SOURCES 		=