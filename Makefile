.PHONY: clean checkstyle format debug

clean: 
	rm -f *.o

checkstyle: 
	clang-tidy --quiet $(wildcard *.c) $(wildcard *.h) --

format:
	clang-format -i $(wildcard *.c) $(wildcard *.h)