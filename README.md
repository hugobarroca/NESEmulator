# NESEmulator

!WARNING: This project is still a work in progress!

This is a personal project, intended to be used for practicing the C language, and programming in general.
It's a (for the time being) naive implementation of a NTCS NES emulator (meaning the Ricoh 2A03 processor).

## How to build this project

You can use cmake to build this project. Dependencies are assumed to be installed on the target system (only tested on Linux) and can be installed by running `getDependencies.sh`.

You can run Cmake as you'd like, or run `./buildAndRun.sh` to build and run the project right away.


## Roadmap
- Main goal: Finish emulator implementation, inluding CPU, APU and PPU, in a state that makes it possible to play some basic Roms decently.
- Add logisim files for the Ricoh2A03 CPU, which can help interactively understand its functionality. 
