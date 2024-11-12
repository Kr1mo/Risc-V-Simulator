# Risc-V-Simulator

This repository aims to simulate the behavior of an RISV-V processor.
Given an input state, it shall simulate a processor and return the new state post simulation.

A state is represented by a .state file.
In there, all initialised registers and memory is stored.
it has the form of
    REGISTERS:
    (name):(value)
    ...

    MEMORY:
    (starting_adress):(memory_representation)
    ...

name is the registers name
value is a 64bit number in hex, split into 4 chunks of 2byte

starting_adress is a 32bit number in hex, split into 2 chunks of 2byte
memory_representation is either a 32bit or 64 bit number in hex, split in chunks of 2byte

Work in progress, details TODO


