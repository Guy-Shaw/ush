
LIBRARY := libush
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

CC := gcc
CFLAGS += -std=c99 -Wall -Wextra -g -fPIC
CPPFLAGS := -I../inc

.PHONY: all clean

all: $(LIBRARY).a

$(LIBRARY).a: $(OBJECTS)
	ar crv $(LIBRARY).a $(OBJECTS)

clean:
	rm -f $(LIBRARY).a $(OBJECTS) *.o
	cscope-clean

