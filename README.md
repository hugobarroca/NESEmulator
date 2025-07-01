# NESEmulator

!WARNING: This project is still a work in progress!

This is a personal project, intended to be used for practicing the C language, and programming in general.
It's a (for the time being) naive implementation of a NTCS NES emulator (meaning the Ricoh 2A03 processor).

## How to run this project

First, make sure you have a C compiler like GCC installed on your machine, to compile the source code.
Using gcc, you can use the command:
`gcc main.c utilities.c cpu.c - emulator.out`

This will create an emulator.out file in the project directory, which you can then run (in a linux system) like:
`./emulator.out`

## Roadmap
- Main goal: Finish emulator implementation, inluding CPU, APU and PPU, in a state that makes it possible to play some basic Roms decently.
- Add logisim files for the Ricoh2A03 CPU, which can help interactively understand its functionality. 
