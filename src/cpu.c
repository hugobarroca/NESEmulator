// This file is meant to hold all the necessary code to emulate the Ricoh 2A03 CPU (based on the 6502 CPU).
#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"

void initProcessor(CPU *cpu){
	cpu->A = 0;
	cpu->X = 0;
	cpu->Y = 0;
	cpu->P = 0;
	cpu->S = 0xFF;
	cpu->PC = 0;

	// The stack lives in addresses 0x0100 to 0x01FF
	for(int i = 0x0100; i < 0x1FF; i++){
		cpu->Memory[i] = 0;
	}
}

void pushStack(CPU *cpu, uint8_t value){
	if(cpu->S < 0x00){
		printf("ERROR; Stack overflow detected!");
	}
	cpu->Memory[cpu->S + 0x0100] = value;
	cpu->S--;	
}


uint8_t popStack(CPU *cpu){
	if(cpu->S == 0xFF){
		printf("ERROR: Stack underflow detected!");
	}
	cpu->S++;
	return cpu->Memory[(cpu->S - 1) + 0x0100];
}

uint8_t readBus(CPU *cpu, uint16_t address){
	// Get it from zero-page memory.
	if(address >= 0x00 && address <= 0x00FF){
		
	} 
	// Get it from the stack.
	if(address >= 0x0100 && address <= 0x01FF){
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
	case(0x05):
		// orAZeroPage(cpu);
		break;
	case(0x15):
		// orAZeroPageX(cpu);
		break;
	case(0x0D):
		// orAAbsolute(cpu);
		break;
	case(0x1D):
		// orAAbsoluteX(cpu);
		break;
	case(0x19):
		// orAbsoluteY(cpu);
		break;
	case(0x01):
		// orAIndirectX(cpu);
		break;
	case(0x11):
		// orAIndirectY(cpu);
		break;
	default:
		break;
	}
}

