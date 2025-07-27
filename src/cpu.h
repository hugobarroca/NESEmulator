#include <stdint.h>

typedef struct CPU CPU;
typedef uint8_t (*ReadBus)(CPU *, uint16_t);

struct CPU {
  // Accumulator
  uint8_t A;
  // Register X
  uint8_t X;
  // Register Y
  uint8_t Y;
  // Processor Status
  // 7 Negative Flag (N)
  // 6 Overflow Flag (V)
  // 5 Unused Flag
  // 4 B Flag (Mostly unused) Flag
  // 3 Decimal Mode Flag (D)
  // 2 Interrupt Disable (I)
  // 1 Zero Flag (Z)
  // 0 Carry Flag (C)
  uint8_t P;
  uint8_t S;
  // Program Counter
  uint16_t PC;
  // 64KiB, full address space, with the following mapping:
  // 0x0000-0x07FF is the actual RAM addresses, and then they are mirrored 3
  // times, till 0x1FFF
  uint8_t Memory[65536];
  // Emulator specific fields
  uint8_t *GameData;
  uint8_t MapperType;
  ReadBus ReadBus;
};

void initProcessor(CPU *cpu);
uint8_t getStackPointerValue(CPU *cpu);
uint8_t getCurrentInstruction(CPU *cpu);
void setAndPrintMapper(CPU *cpu, uint8_t mapperNumber);
void execute(CPU *cpu);
