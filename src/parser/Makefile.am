AM_CPPFLAGS = -I$(top_srcdir)/include
# overwrite Werror for warning created by yacc/lex
AM_CFLAGS = @GPROM_CFLAGS_OPT@ 

BUILT_SOURCES = sql_parser.lex.h sql_parser.tab.h postgres_parser.lex.h postgres_parser.tab.h dl_parser.tab.h dl_parser.lex.h jp_parser.tab.h jp_parser.lex.h
AM_YFLAGS = -d
CLEANFILES = sql_parser.lex.h sql_parser.lex.c sql_parser.tab.h sql_parser.tab.c \
			postgres_parser.lex.h postgres_parser.lex.c postgres_parser.tab.h postgres_parser.tab.c \
			dl_parser.lex.h dl_parser.lex.c dl_parser.tab.h dl_parser.tab.c \
                        jp_parser.lex.h jp_parser.lex.c jp_parser.tab.h jp_parser.tab.c 

noinst_LTLIBRARIES        	= libparser.la
libparser_la_SOURCES		= parser.c \
			          parser_oracle.c sql_parser.lex.c sql_parser.tab.c \
				  parser_postgres.c postgres_parser.lex.c postgres_parser.tab.c \
			          parser_dl.c dl_parser.lex.c dl_parser.tab.c \
                                  parser_jp.c jp_parser.lex.c jp_parser.tab.c

#parser_hive.c hive_parser.lex.c hive_parser.tab.c 

libparser_la_LIBADD      	= 

sql_parser.lex.h sql_parser.lex.c: sql_parser.l
	$(LEX) -osql_parser.lex.c $<

sql_parser.tab.h sql_parser.tab.c: sql_parser.y
	bison -Werror -d $<
	
postgres_parser.lex.h postgres_parser.lex.c: postgres_parser.l
	$(LEX) -opostgres_parser.lex.c $<

postgres_parser.tab.h postgres_parser.tab.c: postgres_parser.y
	bison -b postgres_parser -Werror -d $<

#hive_parser.lex.h hive_parser.lex.c: hive_parser.l
#	$(LEX) -ohive_parser.lex.c $<

#hive_parser.tab.h hive_parser.tab.c: hive_parser.y
#	bison -b hive_parser -Werror -d $<

dl_parser.lex.h dl_parser.lex.c: dl_parser.l
	$(LEX) -odl_parser.lex.c $<

dl_parser.tab.h dl_parser.tab.c: dl_parser.y
	bison -b dl_parser -Werror -d $<

jp_parser.lex.h jp_parser.lex.c: jp_parser.l
	$(LEX) -ojp_parser.lex.c $<

jp_parser.tab.h jp_parser.tab.c: jp_parser.y
	bison -b jp_parser -Werror -d $<
