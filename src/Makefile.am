AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

SUBDIRS = configuration 		\
		log			 			\
		mem_manager				\
		instrumentation			\
		model					\
		utility					\
		provenance_rewriter		\
		parser 					\
		analysis_and_translate 	\
		sql_serializer 			\
		metadata_lookup 		\
		operator_optimizer 		\
		execution				\
		. 						\
		command_line

noinst_LTLIBRARIES 		= libsrc.la
libsrc_la_LIBADD 		= 	configuration/libconfiguration.la \
						log/liblog.la \
						mem_manager/libmem_manager.la \
						model/libmodel.la \
						provenance_rewriter/libprovenance_rewriter.la \
						parser/libparser.la \
						analysis_and_translate/libanalysis.la \
						sql_serializer/libserializer.la \
						metadata_lookup/libmetadata_lookup.la \
						operator_optimizer/liboperator_optimizer.la \
						instrumentation/libinstrumentation.la \
						execution/libexecution.la \
						utility/libutils.la 

libsrc_la_SOURCES 		= rewriter.c