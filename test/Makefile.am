AM_CPPFLAGS = -I$(top_srcdir)/include -I.

bin_PROGRAMS = testmain testparser testupana testanalysis testtranslate testserializer

testmain_SOURCES = test_main.c test_copy.c test_equal.c test_expr.c \
		   test_logger.c test_mem_mgr.c test_to_string.c \
		   test_parse.c test_list.c test_main.h \
		   test_metadata_lookup.c test_string.c 

testmain_LDADD = ../src/libsrc.la

testparser_SOURCES = test_parser.c
testparser_LDADD = ../src/libsrc.la

testupana_SOURCES = test_update_analysis.c
testupana_LDADD = ../src/libsrc.la

testanalysis_SOURCES = test_analysis.c
testanalysis_LDADD = ../src/libsrc.la

testtranslate_SOURCES = test_translate.c
testtranslate_LDADD = ../src/libsrc.la

testserializer_SOURCES = test_serializer.c
testserializer_LDADD = ../src/libsrc.la