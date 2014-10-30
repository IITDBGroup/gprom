AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

noinst_LTLIBRARIES        		= libserializer.la
libserializer_la_SOURCES       	= sql_serializer.c sql_serializer_oracle.c query_block_to_sql.c
libserializer_la_LIBADD        	= 