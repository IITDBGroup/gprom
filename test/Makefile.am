AM_CPPFLAGS = -I$(top_srcdir)/include -I.

<<<<<<< HEAD
bin_PROGRAMS = testmain testparser testupana testanalysis testtranslate testserializer
=======
bin_PROGRAMS = testmain testparser testupana testanalysis testtranslate testvisit
>>>>>>> 04f2886a1dad91873039f06a6ef9295637759171

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

<<<<<<< HEAD
testserializer_SOURCES = test_serializer.c
testserializer_LDADD = ../src/libsrc.la
=======
testvisit_SOURCES = test_visit.c
testvisit_LDADD = ../src/libsrc.la
>>>>>>> 04f2886a1dad91873039f06a6ef9295637759171
