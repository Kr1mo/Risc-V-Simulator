# Risc-V-Simulator

This repository aims to simulate the behavior of an RISV-V processor.
Given an input state, it shall simulate a processor and return the new state post simulation.


## Options
The compiled risc_v_sim takes following arguments:
-e < filename of output > (optional)

-c < number of cycles of the simulation > (optional) TODO

filename of the initial state


## States
A state is represented by a .state file.
In there, all initialised registers and memory is stored.
it has the form of
    REGISTERS:
    (name):(value)
    ...

    MEMORY:
    (starting_adress):(memory_representation)
    ...

name is the registers name x0-x31
value is a 64bit number in hex

starting_adress is a 32bit number in hex
memory_representation is either a 32bit or 64 bit number in hex
