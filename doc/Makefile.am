AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

man1_MANS				= gprom.man

GPROM_MAN=gprom.man
GPROM_MAN_HTML=gprom_man.html
GPROM_MAN_MD=gprom_man.md

# create markdown page from gprom manual page
if HAVE_PROG_PANDOC
if HAVE_PROG_RMAN 
all-local: $(GPROM_MAN_MD)

$(GPROM_MAN_MD): $(GPROM_MAN_HTML)
	pandoc -f html -t markdown_github $(GPROM_MAN_HTML) -o $(GPROM_MAN_MD)

$(GPROM_MAN_HTML): $(GPROM_MAN)
	rman -S -f HTML gprom.man | sed -e 's/[*]*//g' > gprom_man.html

clean-local:
	rm $(GPROM_MAN_HTML) $(GPROM_MAN_MD)
endif
endif