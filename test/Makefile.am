AM_CPPFLAGS = -I$(top_srcdir)/include -I.

bin_PROGRAMS = testmain testparser

testmain_SOURCES = test_main.c test_copy.c test_equal.c test_expr.c \
					test_logger.c test_mem_mgr.c test_to_string.c \
					test_parse.c test_list.c test_main.h
testmain_LDADD = ../src/libsrc.la

testparser_SOURCES = test_parser.c
testparser_LDADD = ../src/libsrc.la