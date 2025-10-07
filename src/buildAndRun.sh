#!/bin/bash
if ! [ -d "build" ];
then
				mkdir build
else
				echo "Build directory exists."
fi
cd build && cmake .. && cmake --build . && ./emulator.out
