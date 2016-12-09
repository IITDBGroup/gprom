AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

man1_MANS				= gprom.man

GPROM_MAN=gprom.man
GPROM_MAN_HTML=gprom_man.html
GPROM_MAN_MD=gprom_man.md

CLEANFILES = $(GPROM_MAN_HTML) $(GPROM_MAN_MD)

if HAVE_PROG_RMAN 

BUILT_SOURCES=create-html

.PHONY = create-html

create-html:
	@if test -f  gprom_man.html; then :; else \
		echo "Generate html manual"; \
		rman -S -f HTML  gprom.man | sed -e 's/[*]*//g' >  gprom_man.html; \
	fi

endif


# create markdown page from gprom manual page
if HAVE_PROG_PANDOC
if HAVE_PROG_RMAN 
all-local: $(GPROM_MAN_MD)

$(GPROM_MAN_MD): $(GPROM_MAN_HTML)
	pandoc -f html -t markdown_githu $(GPROM_MAN_HTML) -o $(GPROM_MAN_MD) || (echo "pandoc with markdown_github failed $$?")
	@if test -f $(GPROM_MAN_MD); then :; else \
		pandoc -f html -t markdown $(GPROM_MAN_HTML) -o $(GPROM_MAN_MD); \
	fi

$(GPROM_MAN_HTML): $(GPROM_MAN)
	rman -S -f HTML $(GPROM_MAN) | sed -e 's/[*]*//g' > $(GPROM_MAN_HTML)

CLEAN_FILES = $(GPROM_MAN_HTML) $(GPROM_MAN_MD)

endif
endif