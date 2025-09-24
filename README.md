# Risc-V-Simulator

This repository aims to simulate the behavior of an RISV-V processor.
Given an input state, it shall simulate a processor and return the new state post simulation.


## Usage
compile with 'make compile', then run with:
./risc_v_sim (options) (path of initial state)


## Options
The compiled risc_v_sim takes following arguments:
-e (filename of output)
default is 'states/end.state'

-n (number of commands executed)
default is 1, with -1, execution is stopped when pc reaches non-initialised memory

-l
prints the last state in the terminal

-d
prints every new state until end of execution beginning with the loaded state


## States
A state is represented by a .state file.
In there, all initialised registers and memory is stored.
it has the form of
    REGISTERS:
    (name):(value)
    ...
    (exactly one empty line)
    MEMORY:
    (starting_adress):(memory_representation)
    ...

(name) is the registers name x0-x31
(value) is a 64bit number in hex

(starting_adress) is an up to 64bit number in hex
(memory_representation) is either a 8bit, 16bit, 32bit or 64 bit number in hex

- It is advised that no line is longer than 80 characters.
- Spaces can be placed as wished.
- Empty lines exept the one defined are not possible.
  This includes lines with only a comment.
- Comments can be added to .state files by '#'.
- Using ':' in comments is not possible.
