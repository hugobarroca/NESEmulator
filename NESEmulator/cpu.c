// This file is meant to hold all the necessary code to emulate the Ricoh 2A03 CPU (based on the 6502 CPU).
#include <stdbool.h>
#include "stack.h"
#include "cpu.h"

void initProcessor(CPU *cpu){
	CPU cpu;
	cpu.A = 0;
	cpu.X = 0;
	cpu.Y = 0;
	cpu.P = 0;
	cpu.S = 0;
	cpu.PC = 0;
	for(i = 0; i < 256; i++){
		cpu.stack[i] = 0;
	}
}

void executeInstruction(CPU *cpu){
	//TODO
	return -1;
}

