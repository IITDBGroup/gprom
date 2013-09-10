CC=gcc
CFLAGS=-c -Wall -Iinclude
LDFLAGS=
SOURCES=src/CommandLine/CommandLine.c src/Configuration/Option.c src/Configuration/OptionParser.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test/CommandLine/CommandLine

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf src/CommandLine/*o src/Configuration/*o $(EXECUTABLE)
