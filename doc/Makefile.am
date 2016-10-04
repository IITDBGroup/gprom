AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

man1_MANS				= gprom.man

GPROM_MAN=gprom.man
GPROM_MAN_HTML=gprom_man.html

# create HTML page from gprom manual page
all-local: $(GPROM_MAN_HTML)

$(GPROM_MAN_HTML): $(GPROM_MAN)
	cat $(GPROM_MAN) | groff -mandoc -Thtml > $(GPROM_MAN_HTML)

clean-local:
	rm $(GPROM_MAN_HTML) 