AM_CPPFLAGS = -I$(top_builddir)/include
AM_CFLAGS = @GPROM_CFLAGS@

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

ANT_BUILDFILE=@GPROM_TOP_SRCDIR@/build.xml
GPROM_JDBC_JAR=$(top_builddir)/jdbcbuild/gprom-jdbc.jar
GPROM_LIBRARY=$(top_builddir)/src/libgprom/libgprom.la
JAVA_JDBC_SRC=$(call rwildcard,java,*.java)

if JAVABINDING
# run antbuild if necessary (library has changed or java files have changed or jar file has not been build yet)
all-local: $(GPROM_JDBC_JAR)

$(GPROM_JDBC_JAR): $(JAVA_JDBC_SRC) $(GPROM_LIBRARY)
	ant -f $(ANT_BUILDFILE) jar

# clean should also run ant's clean task
#clean-local:
#	ant -f $(ANT_BUILDFILE) distClean
endif
