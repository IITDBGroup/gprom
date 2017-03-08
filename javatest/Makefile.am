AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = @GPROM_CFLAGS@

ANT_BUILDFILE=$(top_srcdir)/blackboxtests/build.xml
GPROM_JDBC_TEST_JAR=$(top_srcdir)/build/gprom-jdbc-test.jar
GPROM_JDBC_JAR=$(top_srcdir)/build/gprom-jdbc.jar
#JAVA_TEST_SRC=$(wildcard $(top_srcdir)/javatest/*.java)
JAVA_TEST_SRC=$(shell find $(top_srcdir)/javatest -type f -name '*.java')
TEST_XML_SRC=$(wildcard $(top_srcdir)/blackboxtests/testcases/*.xml)

if JAVABINDING

# run antbuild if necessary (library has changed or java files have changed or jar file has not been build yet)
all-local: $(GPROM_JDBC_TEST_JAR)

$(GPROM_JDBC_TEST_JAR): $(JAVA_TEST_SRC) $(TEST_XML_SRC)
	ant -f $(ANT_BUILDFILE) compileTests

# clean should also run ant's clean task
clean-local:
	ant -f $(ANT_BUILDFILE) distClean

endif