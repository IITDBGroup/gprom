AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

noinst_LTLIBRARIES 			  	= libmetadata_lookup.la
libmetadata_lookup_la_SOURCES	= metadata_lookup.c metadata_lookup_oracle.c \
								metadata_lookup_postgres.c metadata_lookup_external.c \
								metadata_lookup_sqlite.c metadata_lookup_duckdb.c metadata_lookup_monetdb.c \
								metadata_lookup_odbc.c metadata_lookup_mssql.c
