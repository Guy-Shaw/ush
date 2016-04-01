
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
PROGRAMS := $(patsubst %.c,%,$(SOURCES))
DISTBIN := /usr/local/bin

CC := clang
CPPFLAGS := -I../inc
CFLAGS := -g

LIBCSCRIPT := ../libcscript/libcscript.a

.PHONY: all cscope clean install show-targets

all: $(PROGRAMS)

$(PROGRAMS): $(OBJECTS) ../libush/libush.a $(LIBCSCRIPT) -lexplain

../libush/libush.a:
	cd ../libush && make libush.a

../libcscript/libcscript.a:
	cd ../libcscript && make libcscript.a

valgrind:
	valgrind --leak-check=full ./ush --fork ls-script-text

cscope:
	cscope-init

clean:
	cd ../libush && make clean
	rm -f $(PROGRAMS) *.o

install: all
	cp $(PROGRAMS) $(DISTBIN)
	cd $(DISTBIN) && chmod 755 $(PROGRAMS)

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)

#END
