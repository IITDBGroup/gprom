AM_CPPFLAGS = -I$(top_srcdir)/include

BUILT_SOURCES =
AM_YFLAGS = -d

noinst_LTLIBRARIES        	= libparser.la
libparser_la_SOURCES		= parser.c parser_oracle.c sql_parser.lex.c sql_parser.tab.c
libparser_la_LIBADD      	= 

sql_parser.lex.h sql_parser.lex.c: sql_parser.l
	$(LEX) -osql_parser.lex.c $<

sql_parser.tab.h sql_parser.tab.c: sql_parser.y
	bison -Werror -d $<