AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

noinst_LTLIBRARIES        		= libserializer.la
libserializer_la_SOURCES       	= sql_serializer.c sql_serializer_oracle.c sql_serializer_dl.c sql_serializer_lb.c\
								  sql_serializer_sqlite.c sql_serializer_duckdb.c sql_serializer_postgres.c sql_serializer_common.c \
								  query_block_to_sql.c
libserializer_la_LIBADD        	= 