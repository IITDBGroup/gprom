AM_CPPFLAGS = -I$(top_srcdir)/include

SUBDIRS = expression helperfunction list node

noinst_LTLIBRARIES		= libmodel.la
libmodel_la_LIBADD     	= expression/libexpression.la \
						helperfunction/libhelperfunction.la \
						list/liblist.la \
						node/libnode.la
libmodel_la_SOURCES     =