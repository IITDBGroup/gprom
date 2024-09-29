AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

noinst_LTLIBRARIES        		= libupdate_ps.la
libupdate_ps_la_SOURCES		    = update_ps_main.c \
                                update_ps_incremental.c \
                                update_ps_build_state.c \
                                store_operator_data.c \
                                fetch_operator_data.c \
                                rbtree.c \
								bloom.c \
								MurmurHash2.c \
                                base64.c \
                                sketch_based_checking.c
