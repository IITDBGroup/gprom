AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

noinst_LTLIBRARIES        		= libanalysis.la
libanalysis_la_SOURCES       	= translator.c translator_oracle.c translator_dl.c translate_update.c translate_nautilus.c \
	                              analyzer.c analyze_oracle.c analyze_dl.c analyze_nautilus.c \
                                  parameter.c
libanalysis_la_LIBADD        	=
