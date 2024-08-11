// This file is meant to hold all the necessary code to emulate the Ricoh 2A03 CPU (based on the 6502 CPU).
#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"

// The stack lives in addresses 0x0100 to 0x01FF

void initProcessor(CPU *cpu){
	cpu->A = 0;
	cpu->X = 0;
	cpu->Y = 0;
	cpu->P = 0;
	cpu->S = 0xFF;
	cpu->PC = 0;
	for(int i = 0; i < 256; i++){
		cpu->stack[i] = 0;
	}
}

void pushStack(CPU *cpu, uint8_t value){
	if(cpu->S < 0x00){
		printf("ERROR; Stack overflow detected!");
	}
	cpu->stack[cpu->S] = value;
	cpu->S--;	
}


uint8_t popStack(CPU *cpu){
	if(cpu->S == 0xFF){
		printf("ERROR: Stack underflow detected!");
	}
	cpu->S++;
	return cpu->stack[cpu->S - 1];
}

uint8_t readBus(CPU *cpu, uint16_t address){
	if(address >= 0x0100 && address <= 0x01FF){
		// Get it from the stack
		return popStack(cpu);
	}
	printf("Invalid bus address was accessed!");
	return -1;
}

void orAImmediate(CPU *cpu){
	uint8_t immediateValue = readBus(cpu, cpu->PC);
	cpu->PC++;
	cpu->A = cpu->A | immediateValue;
}

void executeInstruction(CPU *cpu){
	//Read instruction from the program counter
	uint8_t instruction = readBus(cpu, cpu->PC);
	cpu->PC++;
	 
	switch (instruction){
	case(0x09):
		orAImmediate(cpu);
		break;
	default:
		break;
	}
}

