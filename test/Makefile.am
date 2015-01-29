AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@ -I.

bin_PROGRAMS = testmain testparser testupana testanalysis testtranslate \
				testserializer testvisit testrewriter testpicsrewrite

testmain_SOURCES = test_main.c test_copy.c test_equal.c test_expr.c \
		   test_logger.c test_mem_mgr.c test_to_string.c \
		   test_parse.c test_list.c test_main.h \
		   test_metadata_lookup.c test_string.c test_set.c test_hashmap.c \
		   test_graph_pi_cs.c \
		   test_vector.c test_parameter.c test_metadata_postgres.c test_string_utils.c \
		   test_dl.c test_hash.c

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

testvisit_SOURCES = test_visit.c
testvisit_LDADD = ../src/libsrc.la

testrewriter_SOURCES = test_rewriter.c
testrewriter_LDADD = ../src/libsrc.la

testpicsrewrite_SOURCES = test_pi_cs_rewrite.c
testpicsrewrite_LDADD = ../src/libsrc.la
