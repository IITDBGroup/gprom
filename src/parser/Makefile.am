AM_CPPFLAGS = -I$(top_srcdir)/include

BUILT_SOURCES = sql_parser.h
AM_YFLAGS = -d

noinst_LTLIBRARIES        	= libparser.la
libparser_la_SOURCES		= parser.c sql_parser.tab.c sql_parser.lex.c sql_parser.y sql_parser.l
libparser_la_LIBADD      	= 