
LIBS := ../../libush/libush.a /usr/local/lib/cscript/libcscript.a

run: test-date
	./test-date

test-date: test-date.c
	clang -o $@ $^ $(LIBS)

clean:
	rm -f test-date tmp-*
