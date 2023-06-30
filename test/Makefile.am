AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/test
AM_CFLAGS = @GPROM_CFLAGS@
# AM_CFLAGS = @GPROM_CFLAGS@
AM_CPPFLAGS += -DGPROM_TOPDIR=\"$(top_srcdir)/\"


check_PROGRAMS = testmain testparser testupana testanalysis testtranslate \
				testserializer testvisit testrewriter testpicsrewrite \
				testrpq testlibgprom testuncert
bin_PROGRAMS = testexprcplex

# testcplex

testmain_SOURCES = test_main.c test_copy.c test_equal.c test_expr.c \
		   test_logger.c test_mem_mgr.c test_to_string.c \
		   test_parse.c test_list.c test_main.h \
		   test_metadata_lookup.c test_string.c test_set.c test_hashmap.c \
		   test_vector.c test_parameter.c test_metadata_postgres.c test_string_utils.c \
		   test_dl.c test_hash.c test_exception.c test_rpq.c test_common.c \
		   test_autocast.c test_bitset.c test_merge_operators.c

testmain_LDADD = $(top_builddir)/src/libsrc.la

testexprcplex_SOURCES = test_expr_to_cplex.c
testexprcplex_LDADD = $(top_builddir)/src/libsrc.la

# testcplex_SOURCES = test_cplex.c
# testcplex_LDADD = $(top_builddir)/src/libsrc.la

testparser_SOURCES = test_parser.c
testparser_LDADD = $(top_builddir)/src/libsrc.la

testuncert_SOURCES = test_uncert.c
testuncert_LDADD = $(top_builddir)/src/libsrc.la

testupana_SOURCES = test_update_analysis.c
testupana_LDADD = $(top_builddir)/src/libsrc.la

testanalysis_SOURCES = test_analysis.c
testanalysis_LDADD = $(top_builddir)/src/libsrc.la

testtranslate_SOURCES = test_translate.c
testtranslate_LDADD = $(top_builddir)/src/libsrc.la

testserializer_SOURCES = test_serializer.c
testserializer_LDADD = $(top_builddir)/src/libsrc.la

testvisit_SOURCES = test_visit.c
testvisit_LDADD = $(top_builddir)/src/libsrc.la

testrewriter_SOURCES = test_rewriter.c
testrewriter_LDADD = $(top_builddir)/src/libsrc.la

testpicsrewrite_SOURCES = test_pi_cs_rewrite.c
testpicsrewrite_LDADD = $(top_builddir)/src/libsrc.la

testrpq_SOURCES = test_rpq_to_dl.c
testrpq_LDADD = $(top_builddir)/src/libsrc.la

testlibgprom_SOURCES = test_libgprom.c test_common.c
testlibgprom_LDADD = $(top_builddir)/src/libgprom/libgprom.la $(top_builddir)/src/libsrc.la


# running test cases with make check
TESTS = testmain
