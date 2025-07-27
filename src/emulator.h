#include "cpu.h"

void detectGameFormat(CPU *cpu);
void printMapperName(uint8_t mapperNumber);
void readGameHeader(CPU *cpu);
void* loadGame(CPU *cpu, char fileName[]);
