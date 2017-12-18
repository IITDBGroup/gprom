AM_CPPFLAGS = -I$(top_builddir)/include
AM_CFLAGS = @GPROM_CFLAGS@

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

ANT_BUILDFILE=./build.xml
GPROM_JNA_FILE=$(top_builddir)/src/interfaces/jdbc/java/org/gprom/jdbc/jna/GProM_JNA.java
GPROM_LIBRARY_HEADER=$(top_builddir)/include/libgprom/libgprom.h
JAVA_GENERATOR_SRC=$(call rwildcard,src,*.java)

if JAVABINDING
# run antbuild if necessary (library has changed or java files have changed or jar file has not been build yet)
all-local: $(GPROM_JNA_FILE)

$(GPROM_JNA_FILE): $(JAVA_GENERATOR_SRC) $(GPROM_LIBRARY_HEADER) 
	ant -f $(ANT_BUILDFILE)

# clean should also run ant's clean task
#clean-local:
#	ant -f $(ANT_BUILDFILE) distClean
endif
