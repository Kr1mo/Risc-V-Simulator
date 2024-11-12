CFLAGS= -fsanitize=address -g -Wall
.PHONY: clean checkstyle format debug

clean: 
	rm -f *.o risc_v_sim

checkstyle: 
	clang-tidy $(wildcard *.c) $(wildcard *.h)

format:
	clang-format -i $(wildcard *.c) $(wildcard *.h)

compile: risc_v_sim

risc_v_sim: risc_v_sim.o state.o processor.o
	gcc $(CFLAGS) risc_v_sim.o state.o processor.o -o risc_v_sim

risc_v_sim.o: risc_v_sim.c state.h processor.h
	gcc $(CFLAGS) -c risc_v_sim.c -o risc_v_sim.o

state.o: state.c state.h
	gcc $(CFLAGS) -c state.c -o state.o

processor.o: processor.c processor.h state.h
	gcc $(CFLAGS) -c processor.c -o processor.o