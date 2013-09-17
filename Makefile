CC=gcc
CFLAGS=-c -Wall -Iinclude
LDFLAGS=
SRCDIR=./src
TESTDIR=./test
SOURCES=$(foreach x, $(SRCDIR), $(wildcard $(addprefix ${x}/, *.c */*.c */*/*.c)))
TESTSRC=$(foreach x, $(TESTDIR), $(wildcard $(addprefix ${x}/, *.c */*.c */*/*.c))) 
OBJECTS=$(SOURCES:.c=.o) $(TESTSRC:.c=.o)
CMD_OBJS=$(SOURCES:.c=.o)
TEST_OBJS=$(filter-out ./src/command_line/command_line_main.o, $(OBJECTS))
CMD=test/CommandLine/CommandLine
TEST=test/test

all: $(CMD) $(TEST)

$(CMD): $(CMD_OBJS)
	$(CC) $(LDFLAGS) $(CMD_OBJS) -o $@

$(TEST): $(TEST_OBJS) 
	$(CC) $(LDFLAGS) $(TEST_OBJS) -o $@

$(OBJECTS): %.o : %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean: 
	rm -rf src/*.o src/*/*.o src/*/*/*.o test/*.o $(CMD) $(TEST)
