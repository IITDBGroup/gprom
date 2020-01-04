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
if HAVE_PROG_PANDOC_WITH_MAN

all-local: $(GPROM_MAN_MD)

$(GPROM_MAN_HTML): $(GPROM_MAN)
	$(PANDOC) -f man $(GPROM_MAN) -o $(GPROM_MAN_HTML)
# sed -e 's/[*]*//g' > $(GPROM_MAN_HTML)

$(GPROM_MAN_MD): $(GPROM_MAN_HTML)
	$(PANDOC) -f html -t markdown_github $(GPROM_MAN_HTML) -o $(GPROM_MAN_MD) || (echo "pandoc with markdown_github failed $$?")
	@if test -f $(GPROM_MAN_MD); then :; else \
		$(PANDOC) -f html -t markdown $(GPROM_MAN_HTML) -o $(GPROM_MAN_MD); \
	fi

CLEAN_FILES = $(GPROM_MAN_HTML) $(GPROM_MAN_MD)

endif
endif

# only use rman if we do not have pandoc
if !HAVE_PROG_PANDOC_WITH_MAN
if HAVE_PROG_RMAN

all-local: $(GPROM_MAN_HTML)

$(GPROM_MAN_HTML): $(GPROM_MAN)
	rman -S -f HTML $(GPROM_MAN) | sed -e 's/[*]*//g' > $(GPROM_MAN_HTML)

CLEAN_FILES = $(GPROM_MAN_HTML) $(GPROM_MAN_MD)

endif
endif
