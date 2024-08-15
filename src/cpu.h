#include <stdint.h>

typedef struct {
	uint8_t A;
	uint8_t X;
	uint8_t Y;
	uint8_t P;
	uint8_t S;
	uint16_t PC;
	uint8_t Memory[65536];
} CPU;

void loadGame(char fileName[]);
