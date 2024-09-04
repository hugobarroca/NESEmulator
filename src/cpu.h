#include <stdint.h>

typedef struct {
	uint8_t A;
	uint8_t X;
	uint8_t Y;
	// Processor Status
	// 0 Carry Flag
	// 1 Zero Flag
	// 2 Interrupt Disable
	// 3 Decimal Mode Flag
	// 4 B Flag (Mostly unused) Flag
	// 5 Unused Flag
	// 6 Overflow Flag
	// 7 Negative Flag 
	uint8_t P;
	uint8_t S;
	uint16_t PC;
	// 64KiB, full address space, with the following mapping:
	// 0x0000-0x07FF is the actual RAM addresses, and then they are mirrored 3 times, till 0x1FFF
	uint8_t Memory[65536]; 
} CPU;

void loadGame(CPU *cpu, char fileName[]);
