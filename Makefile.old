CC=gcc
CFLAGS=-c -g -Wall -Iinclude -std=c99
LDFLAGS=
SRCDIR=./src
TESTDIR=./test
INCLUDEDIR=./include
SOURCES=$(foreach x, $(SRCDIR), $(wildcard $(addprefix ${x}/, *.c */*.c */*/*.c)))
TESTSRC=$(foreach x, $(TESTDIR), $(wildcard $(addprefix ${x}/, *.c */*.c */*/*.c)))
HEADERS=$(foreach x, $(INCLUDEDIR), $(wildcard $(addprefix ${x}/, *.h */*.h */*/*.h)))
OBJECTS=$(SOURCES:.c=.o) $(TESTSRC:.c=.o)
CMD_OBJS=$(SOURCES:.c=.o)
TEST_OBJS=$(filter-out ./src/command_line/command_line_main.o, $(OBJECTS))
CMD=test/CommandLine/CommandLine
TEST=test/test

all: $(CMD) 
test: $(TEST)

$(CMD): $(CMD_OBJS) $(HEADERS)
	$(CC) $(LDFLAGS) $(CMD_OBJS) -o $@k

$(TEST): $(TEST_OBJS) $(HEADERS)
	$(CC) $(LDFLAGS) $(TEST_OBJS) -o $@

$(OBJECTS): %.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean: 
	rm -rf src/*.o src/*/*.o src/*/*/*.o test/*.o $(CMD) $(TEST)
