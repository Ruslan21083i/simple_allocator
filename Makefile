CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=
SOURCES=main.c simple_allocator.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=simple_allocator_test

.PHONY: clean all

all: $(SOURCES) $(EXECUTABLE)
	
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
