.PHONY: all .FORCE clean

all: cmd/ush

cmd/ush: .FORCE
	cd libcscript && make
	cd libush && make
	cd cmd && make

clean:
	cd libush && make clean
	cd libcscript && make clean
	cd cmd && make clean

.FORCE:

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
