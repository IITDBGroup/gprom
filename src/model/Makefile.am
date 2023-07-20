AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

SUBDIRS = expression helperfunction list node set graph query_block query_operator datalog rpq bitset integrity_constraints

noinst_LTLIBRARIES		= libmodel.la
libmodel_la_LIBADD     	= expression/libexpression.la \
						helperfunction/libhelperfunction.la \
						list/liblist.la \
						bitset/libbitset.la \
						node/libnode.la \
						set/libset.la \
	                    graph/libgraph.la \
						query_block/libquery_block.la \
						query_operator/libquery_operator.la \
						datalog/libdatalog.la \
						rpq/librpq_model.la \
                        integrity_constraints/libintegrity_constraints.la

libmodel_la_SOURCES     =
