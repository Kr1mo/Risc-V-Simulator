CFLAGS= -O2 -Wall

.PHONY: clean checkstyle format compile debug profile

clean: 
	rm -f *.o risc_v_sim

checkstyle: 
	clang-tidy $(wildcard *.c) $(wildcard *.h)

format:
	clang-format -i $(wildcard *.c) $(wildcard *.h)

compile:clean risc_v_sim

risc_v_sim: risc_v_sim.o state.o processor.o memory_table.o
	gcc $(CFLAGS) risc_v_sim.o state.o processor.o memory_table.o -o risc_v_sim

risc_v_sim.o: risc_v_sim.c state.h processor.h
	gcc $(CFLAGS) -c risc_v_sim.c -o risc_v_sim.o

state.o: state.c state.h memory_table.h
	gcc $(CFLAGS) -c state.c -o state.o

processor.o: processor.c processor.h state.h
	gcc $(CFLAGS) -c processor.c -o processor.o

memory_table.o: memory_table.c memory_table.h
	gcc $(CFLAGS) -c memory_table.c -o memory_table.o

debug: CFLAGS = -O0 -g -Wall
debug: compile

profile: CFLAGS = -O2 -pg -Wall
profile: compile