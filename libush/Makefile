
LIBRARY := libush
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

# CC := clang
# CFLAGS += -I../inc -g3 -fsanitize=undefined

CC := clang
CFLAGS += -g
CPPFLAGS := -I../inc -I/usr/local/include

.PHONY: all clean

all: $(LIBRARY).a

$(LIBRARY).a: $(OBJECTS)
	ar crv $(LIBRARY).a $(OBJECTS)

clean:
	rm -f $(LIBRARY).a $(OBJECTS) *.o
	cscope-clean

