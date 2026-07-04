.PHONY: all clean test

CC := gcc
CFLAGS := -g -Wall -Wextra -std=c11

TARGET := musab-cc
SOURCES := main.c musab_tokenize.c musab_type.c musab_parse.c musab_codegen.c
OBJECTS := $(SOURCES:.c=.o)
DEPS := $(SOURCES:.c=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	$(CC) -MM $(CFLAGS) $< > $@

include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJECTS) $(DEPS) *.s *.o *.out

test: $(TARGET)
	./tests/run_tests.sh
