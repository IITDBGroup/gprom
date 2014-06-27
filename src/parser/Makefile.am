AM_CPPFLAGS = -I$(top_srcdir)/include

BUILT_SOURCES =
AM_YFLAGS = -d

noinst_LTLIBRARIES        	= libparser.la
libparser_la_SOURCES		= parser.c \
							parser_oracle.c sql_parser.lex.c sql_parser.tab.c \
							parser_postgres.c postgres_parser.lex.c postgres_parser.tab.c \
							parser_hive.c hive_parser.lex.c hive_parser.tab.c
libparser_la_LIBADD      	= 

sql_parser.lex.h sql_parser.lex.c: sql_parser.l
	$(LEX) -osql_parser.lex.c $<

sql_parser.tab.h sql_parser.tab.c: sql_parser.y
	bison -Werror -d $<
	
postgres_parser.lex.h postgres_parser.lex.c: postgres_parser.l
	$(LEX) -opostgres_parser.lex.c $<
	
postgres_parser.tab.h postgres_parser.tab.c: postgres_parser.y
	bison -b postgres_parser -Werror -d $<
	
hive_parser.lex.h hive_parser.lex.c: hive_parser.l
	$(LEX) -ohive_parser.lex.c $<
	
hive_parser.tab.h hive_parser.tab.c: hive_parser.y
	bison -b hive_parser -d $<
	