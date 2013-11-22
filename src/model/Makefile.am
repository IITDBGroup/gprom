AM_CPPFLAGS = -I$(top_srcdir)/include

SUBDIRS = expression helperfunction list node set query_block query_operator

noinst_LTLIBRARIES		= libmodel.la
libmodel_la_LIBADD     	= expression/libexpression.la \
						helperfunction/libhelperfunction.la \
						list/liblist.la \
						node/libnode.la \
						set/libset.la \
						query_block/libquery_block.la \
						query_operator/libquery_operator.la #\
						#set/libset.la
						
libmodel_la_SOURCES     =
