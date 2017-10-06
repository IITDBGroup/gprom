AM_CPPFLAGS = -I$(top_builddir)/include
AM_CFLAGS = @GPROM_CFLAGS@

ANT_BUILDFILE=build.xml
GPROM_JDBC_JAR=$(top_builddir)/jdbcbuild/gprom-jdbc.jar
GPROM_LIBRARY=$(top_builddir)/src/libgprom/libgprom_internal.la
JAVA_GEN_SRC=org/gprom/libgen/GenLibGProMH.java
JAVA_GEN_CLASS=$(top_builddir)/javabin/org/gprom/libgen/GenLibGProMH.class
 
if JAVABINDING
# run antbuild if necessary (library has changed or java files have changed or jar file has not been build yet)
all-local: $(JAVA_GEN_CLASS)

$(JAVA_GEN_CLASS): $(JAVA_GEN_SRC) $(GPROM_LIBRARY) 
	ant -f $(ANT_BUILDFILE) genLibraryHeader

# clean should also run ant's clean task
#clean-local:
#	ant -f $(ANT_BUILDFILE) distClean
endif
