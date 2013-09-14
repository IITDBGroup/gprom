CC=gcc
CFLAGS=-c -Wall -Iinclude
LDFLAGS=
SRCDIR=./src
TESTDIR=./test
SOURCES=$(foreach x, $(SRCDIR), $(wildcard $(addprefix ${x}/*, .c /*.c))) $(wildcard $(TESTDIR)/*.c)#src/logger.c src/mem_mgr.c test/test_logger.c test/test_mem_mgr.c test/test.c src/CommandLine/CommandLine.c src/Configuration/Option.c src/Configuration/OptionParser.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test/CommandLine/CommandLine

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf src/*.o src/*/*.o test/*.o $(EXECUTABLE)
