AM_CPPFLAGS = -I@GPROM_TOP_SRCDIR@/include
AM_CFLAGS = @GPROM_CFLAGS@

ANT_BUILDFILE=@GPROM_TOP_SRCDIR@/blackboxtests/build.xml
GPROM_JDBC_TEST_JAR=@GPROM_TOP_SRCDIR@/build/gprom-jdbc-test.jar
GPROM_JDBC_JAR=@GPROM_TOP_SRCDIR@/build/gprom-jdbc.jar
#JAVA_TEST_SRC=$(wildcard @GPROM_TOP_SRCDIR@/javatest/*.java)
JAVA_TEST_SRC=$(shell find @GPROM_TOP_SRCDIR@/javatest -type f -name '*.java')
TEST_XML_SRC=$(wildcard @GPROM_TOP_SRCDIR@/blackboxtests/testcases/*.xml)

if JAVABINDING

# run antbuild if necessary (library has changed or java files have changed or jar file has not been build yet)
all-local: $(GPROM_JDBC_TEST_JAR)

$(GPROM_JDBC_TEST_JAR): $(JAVA_TEST_SRC) $(TEST_XML_SRC)
	ant -f $(ANT_BUILDFILE) compile

# clean should also run ant's clean task
clean-local:
	ant -f $(ANT_BUILDFILE) clean-all

endif
