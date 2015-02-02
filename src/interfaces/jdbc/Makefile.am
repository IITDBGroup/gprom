AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

ANT_BUILDFILE=$(top_srcdir)/build.xml
GPROM_JDBC_JAR=$(top_srcdir)/build/gprom-jdbc.jar

# run antbuild if necessary (library has changed or java files have changed or jar file has not been build yet)
all-local: $(GPROM_JDBC_JAR)

$(GPROM_JDBC_JAR): $(top_srcdir)/src/libgprom/libgprom.la
	ant -f $(ANT_BUILDFILE) generateDriverJar

# clean should also run ant's clean task
clean-local:
	ant -f $(ANT_BUILDFILE) clean