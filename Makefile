.PHONY: all clean

all: cmd/ush

cmd/ush: libush/libush.a  libcscript/libcscript.a
	cd cmd && make

libush/libush.a:
	cd libush && make

libcscript/libcscript.a:
	cd libcscript && make

clean:
	cd libush && make clean
	cd libcscript && make clean
	cd cmd && make clean

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
