// This file is meant to hold all the necessary code to emulate the Ricoh 2A03 CPU (based on the 6502 CPU).
#include <string.h>
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
	for(int i = 0x0100; i < 0x01FF; i++){
		cpu->Memory[i] = 0;
	}
}

void loadGame(CPU *cpu, char fileName[]){
	printf("Attempting to load game: %s\n", fileName);
	FILE *file = fopen(fileName, "rb");
	if(file == NULL){
		printf("File not found.\n");
		return;
	}
	printf("Opened the file successfully.\n");
	uint8_t *gameStart = (cpu->Memory);
	printf("Game start address: %#08x\n", gameStart);
	
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	printf("Filesize: %u bytes\n", fileSize);
	fread(gameStart, 1, (fileSize / 8), file);
	printf("Read file successfully!\n");

	// Read the game header and initialize CPU accordingly
	fclose(file);
}

void pushStack(CPU *cpu, uint8_t value){
	if(cpu->S < 0x00){
		printf("ERROR; Stack overflow detected!\n");
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

// The BUS in the NES had special addresses for different things
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
	uint8_t pcAddr = readBus(cpu, cpu->PC);
	uint8_t immediateValue = readBus(cpu, pcAddr);
	cpu->PC++;
	cpu->A = cpu->A | immediateValue;
}

void orAZeroPage(CPU *cpu){
	uint8_t pcAddr = readBus(cpu, cpu->PC);
	uint8_t address = readBus(cpu, pcAddr);
	cpu->PC++;
	cpu->A = cpu->A | readBus(cpu, (uint16_t)address);
}

void orAZeroPageX(CPU *cpu){
	uint8_t pcAddr =  readBus(cpu, cpu->PC);
	uint8_t pcValue = readBus(cpu, pcAddr);
	cpu->PC++;
	uint16_t address = (uint16_t)pcValue + cpu->X;
	uint8_t memValue = readBus(cpu, address);
	cpu->A = cpu->A | memValue;
}

void orAAbsolute(CPU *cpu){
	uint8_t pcAddr =  readBus(cpu, cpu->PC);
	uint8_t addr1 = readBus(cpu, pcAddr);
	cpu->PC++;
	uint8_t addr2 = readBus(cpu, pcAddr);
	cpu->PC++;
	uint16_t address = (uint16_t)addr1 << 8 | addr2;
	uint8_t memValue = readBus(cpu, address);
	cpu->A = cpu->A | memValue;
}

void executeInstruction(CPU *cpu){
	uint8_t prAddr = readBus(cpu, cpu->PC);
	uint8_t instruction = readBus(cpu, prAddr);
	cpu->PC++;
	switch (instruction){
		case(0x09):
			orAImmediate(cpu);
			break;
		case(0x05):
			orAZeroPage(cpu);
			break;
		case(0x15):
			orAZeroPageX(cpu);
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
