// This file is meant to hold all the necessary code to emulate the Ricoh 2A03
// CPU (based on the 6502 CPU).
#include "cpu.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void initProcessor(CPU *cpu) {
  cpu->A = 0;
  cpu->X = 0;
  cpu->Y = 0;
  cpu->P = 0;
  cpu->S = 0xFF;
  cpu->PC = 0xFFFC;

  // The stack lives in addresses 0x0100 to 0x01FF
  for (int i = 0x0100; i < 0x01FF; i++) {
    cpu->Memory[i] = 0;
  }
}

void readGameHeaders(CPU *cpu) {
  uint8_t *nes = cpu->Memory;
  printf("HEADER START: %.3s\n", nes);
  printf("PRG ROM Size: %d KBs.\n", cpu->Memory[4] * 16);
  printf("CHR ROM Size: %d KBs.\n", cpu->Memory[5] * 8);
  printf("Flags 6: %d\n", cpu->Memory[6]);
  printf("Flags 7: %d\n", cpu->Memory[7]);
  printf("Flags 8: %d\n", cpu->Memory[8]);
  printf("Flags 9: %d\n", cpu->Memory[9]);
  printf("Flags 10: %d\n", cpu->Memory[10]);

  uint8_t *ripper = cpu->Memory + 11;
  printf("RIPPER NAME: %.5s\n", ripper);
}

void pushStack(CPU *cpu, uint8_t value) {
  if (cpu->S < 0x00) {
    printf("ERROR; Stack overflow detected!\n");
  }
  cpu->Memory[cpu->S + 0x0100] = value;
  cpu->S--;
}

uint8_t popStack(CPU *cpu) {
  if (cpu->S == 0xFF) {
    printf("ERROR: Stack underflow detected!");
  }
  cpu->S++;
  return cpu->Memory[(cpu->S - 1) + 0x0100];
}

// The BUS in the NES had special addresses for different things
uint8_t readBus(CPU *cpu, uint16_t address) {
  // Get it from zero-page memory.
  if (address >= 0x00 && address <= 0x00FF) {
  }
  // Get it from the stack.
  if (address >= 0x0100 && address <= 0x01FF) {
    return popStack(cpu);
  }

  return cpu->Memory[address];
  printf("Invalid bus address was accessed!\n");
  return -1;
}

void forceBreak(CPU *cpu) {
  // Push PC + 2 to stack;
  pushStack(cpu, cpu->PC + 2);
  // Push Processor Status to stack with I flag set to 1
  pushStack(cpu, cpu->P | 00000100);
  // Sets PC to be equal to FFFE and FFF0
  cpu->PC = cpu->Memory[0xFFFE];
}
void orAImmediate(CPU *cpu) {
  printf("Executing orAImmediate instruction.");
  uint8_t pcAddr = readBus(cpu, cpu->PC);
  uint8_t immediateValue = readBus(cpu, pcAddr);
  cpu->PC++;
  cpu->A = cpu->A | immediateValue;
}

void orAZeroPage(CPU *cpu) {
  uint8_t pcAddr = readBus(cpu, cpu->PC);
  uint8_t address = readBus(cpu, pcAddr);
  cpu->PC++;
  cpu->A = cpu->A | readBus(cpu, (uint16_t)address);
}

void orAZeroPageX(CPU *cpu) {
  uint8_t pcAddr = readBus(cpu, cpu->PC);
  uint8_t pcValue = readBus(cpu, pcAddr);
  cpu->PC++;
  uint16_t address = (uint16_t)pcValue + cpu->X;
  uint8_t memValue = readBus(cpu, address);
  cpu->A = cpu->A | memValue;
}

void orAAbsolute(CPU *cpu) {
  uint8_t pcAddr = readBus(cpu, cpu->PC);
  uint8_t addr1 = readBus(cpu, pcAddr);
  cpu->PC++;
  uint8_t addr2 = readBus(cpu, pcAddr);
  cpu->PC++;
  uint16_t address = (uint16_t)addr1 << 8 | addr2;
  uint8_t memValue = readBus(cpu, address);
  cpu->A = cpu->A | memValue;
}

void orAAbsoluteX(CPU *cpu) {
  uint8_t pcAddr = readBus(cpu, cpu->PC);
  uint8_t addr1 = readBus(cpu, pcAddr);
  cpu->PC++;
  uint8_t addr2 = readBus(cpu, pcAddr);
  cpu->PC++;
  uint16_t address = (uint16_t)addr1 << 8 | addr2;
  // Get the value of the offset from the X register
  uint8_t offset = readBus(cpu, cpu->X);
  // Add it to the memory address value
  uint16_t finalAddr = address + offset;
  uint8_t memValue = readBus(cpu, finalAddr);
  cpu->A = cpu->A | memValue;
}

void arithmeticShiftLeftZeroPage(CPU *cpu) {
  uint8_t pcAddr = readBus(cpu, cpu->PC);
  uint8_t address = readBus(cpu, pcAddr);
  cpu->PC++;
  uint8_t memValue = readBus(cpu, (uint16_t)address);
  uint8_t shiftResult = memValue << 2;
  cpu->Memory[address] = shiftResult;
}

void executeInstruction(CPU *cpu) {
  uint8_t prAddr = readBus(cpu, cpu->PC);
  uint8_t instruction = readBus(cpu, prAddr);
  cpu->PC++;
  switch (instruction) {
  case (0x00):
    // BRK
    forceBreak(cpu);
    break;
  case (0x01):
    // orAIndirectX(cpu);
    break;
  case (0x05):
    orAZeroPage(cpu);
    break;
  case (0x06):
    // ASL oper, 2 bytes, 5 cycles
    arithmeticShiftLeftZeroPage(cpu);
    break;
  case (0x08):
    // pushProcessorStatusOnStack(cpu);
    break;
  case (0x09):
    orAImmediate(cpu);
    break;
  case (0x0A):
    // arithmeticShiftLeftAccumulator(cpu);
    break;
  case (0x0D):
    orAAbsolute(cpu);
    break;
  case (0x0E):
    // arithmeticShiftLeftAbsolute(cpu);
    break;
  case (0x10):
    // branchOnPlusRelative(cpu);
    break;
  case (0x11):
    // orAIndirectY(cpu);
    break;
  case (0x15):
    orAZeroPageX(cpu);
    break;
  case (0x16):
    // arithmeticShiftLeftZeroPageX(cpu);
    break;
  case (0x18):
    // clearCarry(cpu);
  case (0x19):
    // orAbsoluteY(cpu);
    break;
  case (0x1D):
    orAAbsoluteX(cpu);
    break;
  case (0x1E):
    // arithmeticShiftLeftAbsoluteX(cpu);
    break;
  case (0x20):
    // jumpSubRoutineAbsolute(cpu);
    break;
  case (0x21):
    // andIndirectX(cpu);
    break;
  case (0x24):
    // bitTestZeroPage(cpu);
    break;
  case (0x25):
    // andZeroPage(cpu);
    break;
  case (0x26):
    // rotateLeftZeroPage(cpu);
    break;
  case (0x28):
    // pullProcessorStatusFromStack(cpu);
    break;
  case (0x29):
    // andImmediate(cpu);
    break;
  case (0x2A):
    // rotateLeftAccumulator(cpu);
    break;
  case (0x2C):
    // bitTestAbsolute(cpu);
    break;
  case (0x2D):
    // andAbsolute(cpu);
    break;
  case (0x2E):
    // rotateLeftAbsolute(cpu);
    break;
  case (0x30):
    // branchOnMinusRelative(cpu);
    break;
  case (0x31):
    // andIndirectY(cpu);
    break;
  case (0x35):
    // andZeroPageX(cpu);
    break;
  case (0x36):
    // rotateLeftZeroPageX(cpu);
    break;
  case (0x38):
    // setCarry(cpu);
    break;
  case (0x39):
    // andAbsoluteY(cpu);
    break;
  case (0x3D):
    // andAbsoluteX(cpu);
    break;
  case (0x3E):
    // rotateLeftAbsoluteX(cpu);
    break;
  case (0x40):
    // returnFromInterrupt(cpu);
    break;
  case (0x41):
    // exclusiveOrIndirectX(cpu);
    break;
  case (0x45):
    // exclusiveOrZeroPage(cpu);
    break;
  case (0x46):
    // logisticalShiftRightZeroPage(cpu);
    break;
  case (0x48):
    // pushAccumulatorOntoStack(cpu);
    break;
  case (0x49):
    // exclusiveOrImmediate(cpu);
    break;
  case (0x4A):
    // logisticalShiftRightAccumulator(cpu);
    break;
  case (0x4C):
    // jumpAbsolute(cpu);
    break;
  case (0x4D):
    // exclusiveOrAbsolute(cpu);
    break;
  case (0x4E):
    // logisticalShiftRightAbsolute(cpu);
    break;
  case (0x50):
    // branchOnOverflowClearRelative(cpu);
    break;
  case (0x51):
    // exclusiveOrIndirectY(cpu);
    break;
  case (0x55):
    // exclusiveOrZeroPageX(cpu);
    break;
  case (0x56):
    // logisticalShiftRightZeroPageX(cpu);
    break;
  case (0x58):
    // clearInterruptDisable(cpu);
    break;
  case (0x59):
    // exclusiveOrAbsoluteY(cpu);
    break;
  case (0x5D):
    // exclusiveOrAbsoluteX(cpu);
    break;
  case (0x5E):
    // logisticalShiftRightAbsoluteX(cpu);
    break;
  case (0x60):
    // returnFromSubroutine(cpu);
    break;
  case (0x61):
    // addWithCarryIndirectX(cpu);
    break;
  case (0x65):
    // addWithCarryZeroPage(cpu);
    break;
  case (0x66):
    // rotateRightZeroPage(cpu);
    break;
  case (0x68):
    // pullAccumulatorFromStack(cpu);
    break;
  case (0x69):
    // addWithCarryImmediate(cpu);
    break;
  case (0x6A):
    // rotateRightAccumulator(cpu);
    break;
  case (0x6C):
    // jumpIndirect(cpu);
    break;
  case (0x6D):
    // addWithCarryAbsolute(cpu);
    break;
  case (0x6E):
    // rotateRightAbsolute(cpu);
    break;
  case (0x70):
    // branchonOverflowSetRelative(cpu);
    break;
  case (0x71):
    // addWithCarryIndirectY(cpu);
    break;
  case (0x75):
    // addWithCarryZeroPageX(cpu);
    break;
  case (0x76):
    // rotateRightZeroPageX(cpu);
    break;
  case (0x78):
    // setInterruptDisable(cpu);
    break;
  case (0x79):
    // addWithCarryAbsoluteY(cpu);
    break;
  case (0x7D):
    // addWithCarryAbsoluteX(cpu);
    break;
  case (0x7E):
    // rotateRightAbsoluteX(cpu);
    break;
  case (0x81):
    // storeAccumulatorIndirectX(cpu);
    break;
  case (0x84):
    // storeYZeroPage(cpu);
    break;
  case (0x85):
    // storeAccumulatorZeroPage(cpu);
    break;
  case (0x86):
    // storeXZeroPage(cpu);
    break;
  case (0x88):
    // decrementY(cpu);
    break;
  case (0x8A):
    // transferXToAccumulator(cpu);
    break;
  case (0x8C):
    // storeYAbsolute(cpu);
    break;
  case (0x8D):
    // storeAccumulatorAbsolute(cpu);
    break;
  case (0x8E):
    // storeXAbsolute(cpu);
    break;
  case (0x90):
    // branchOnClearCarryRelative(cpu);
    break;
  case (0x91):
    // storeAccumulatorIndirectY(cpu);
    break;
  case (0x94):
    // storeYZeroPageX(cpu);
    break;
  case (0x95):
    // storeAccumulatorZeroPageX(cpu);
    break;
  case (0x96):
    // storeXZeroPageY(cpu);
    break;
  case (0x98):
    // transferYToAccumulator(cpu);
    break;
  case (0x99):
    // storeAccumulatorAbsoluteY(cpu);
    break;
  case (0x9A):
    // transferXToStackPointer(cpu);
    break;
  case (0x9D):
    // storeAccumulatorAbsoluteX(cpu);
    break;
  case (0xA0):
    // loadYImmediate(cpu);
    break;
  case (0xA1):
    // loadAccumulatorIndirectX(cpu);
    break;
  case (0xA2):
    // loadXImmediate(cpu);
    break;
  case (0xA4):
    // loadYZeroPage(cpu);
    break;
  case (0xA5):
    // loadAccumulatorZeroPage(cpu);
    break;
  case (0xA6):
    // loadXZeroPage(cpu);
    break;
  case (0xA8):
    // transferAccumulatorToY(cpu);
    break;
  case (0xA9):
    // loadAccumulatorImmediate(cpu);
    break;
  case (0xAA):
    // transferAccumulatorToX(cpu);
    break;
  case (0xAC):
    // loadYAbsolute(cpu);
    break;
  case (0xAD):
    // loadAccumulatorAbsolute(cpu);
    break;
  case (0xAE):
    // loadXAbsolute(cpu);
    break;
  case (0xB0):
    // branchOnCarrySetRelative(cpu);
    break;
  case (0xB1):
    // loadAccumulatorIndirectY(cpu);
    break;
  case (0xB4):
    // loadYZeroPageX(cpu);
    break;
  case (0xB5):
    // loadAccumulatorZeroPageX(cpu);
    break;
  case (0xB6):
    // loadXZeroPageY(cpu);
    break;
  case (0xB8):
    // clearOverflow(cpu);
    break;
  case (0xB9):
    // loadAccumulatorAbsoluteY(cpu);
    break;
  case (0xBA):
    // transferStackPointerToX(cpu);
    break;
  case (0xBC):
    // loadYAbsoluteX(cpu);
    break;
  case (0xBD):
    // loadAccumulatorAbsoluteX(cpu);
    break;
  case (0xBE):
    // loadXAbsoluteY(cpu);
    break;
  case (0xC0):
    // compareWithYImmediate(cpu);
    break;
  case (0xC1):
    // compareWithAccumulatorIndirectX(cpu);
    break;
  case (0xC4):
    // compareWithYZeroPage(cpu);
    break;
  case (0xC5):
    // compareWithAccumulatorZeroPage(cpu);
    break;
  case (0xC6):
    // decrementZeroPage(cpu);
    break;
  case (0xC8):
    // incrementY(cpu);
    break;
  case (0xC9):
    // compareWithAccumulatorImmediate(cpu);
    break;
  case (0xCA):
    // decrementX(cpu);
    break;
  case (0xCC):
    // compareWithYAbsolute(cpu);
    break;
  case (0xCD):
    // compareWithAccumulatorAbsolute(cpu);
    break;
  case (0xCE):
    // decrementAbsolute(cpu);
    break;
  case (0xD0):
    // branchOnNotEqualRelative(cpu);
    break;
  case (0xD1):
    // compareWithAccumulatorIndirectY(cpu);
    break;
  case (0xD5):
    // compareWithAccumulatorZeroPageX(cpu);
    break;
  case (0xD6):
    // decrementZeroPageX(cpu);
    break;
  case (0xD8):
    // clearDecimal(cpu);
    break;
  case (0xD9):
    // compareWithAccumulatorAbsoluteY(cpu);
    break;
  case (0xDD):
    // compareWithAccumulatorAbsoluteX(cpu);
    break;
  case (0xDE):
    // decrementAbsoluteX(cpu);
    break;
  case (0xE0):
    // compareWithXImmediate(cpu);
    break;
  case (0xE1):
    // subtractWithCarryIndirectX(cpu);
    break;
  case (0xE4):
    // compareWithXZeroPage(cpu);
    break;
  case (0xE5):
    // subtractWithCarryZeroPage(cpu);
    break;
  case (0xE6):
    // incrementZeroPage(cpu);
    break;
  case (0xE8):
    // incrementX(cpu);
    break;
  case (0xE9):
    // subtractWithCarryImmediate(cpu);
    break;
  case (0xEA):
    // noOperation(cpu);
    break;
  case (0xEC):
    // compareWithXAbsolute(cpu);
    break;
  case (0xED):
    // subtractWithCarryAbsolute(cpu);
    break;
  case (0xEE):
    // incrementAbsolute(cpu);
    break;
  case (0xF0):
    // branchOnEqualRelative(cpu);
    break;
  case (0xF1):
    // subtractWithCarryIndirectY(cpu);
    break;
  case (0xF5):
    // subtractWithCarryZeroPageX(cpu);
    break;
  case (0xF6):
    // incrementZeroPageX(cpu);
    break;
  case (0xF8):
    // setDecimal(cpu);
    break;
  case (0xF9):
    // subtractWithCarryAbsoluteY(cpu);
    break;
  case (0xFD):
    // subtractWithCarryAbsoluteX(cpu);
    break;
  case (0xFE):
    // incrementAbsoluteX(cpu);
    break;
  default:
    break;
  }
}

void execute(CPU *cpu) {
  executeInstruction(cpu);
  printf("Finished executing instrunction. Press any character to continue.\n");
  getchar();
}

void loadGame(CPU *cpu, char fileName[]) {
  printf("Attempting to load game: %s\n", fileName);
  FILE *file = fopen(fileName, "rb");
  if (file == NULL) {
    printf("File not found.\n");
    return;
  }
  printf("Opened the file successfully.\n");
  uint8_t *gameStart = (cpu->Memory);
  printf("Game start address: %8s\n", gameStart);

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  printf("Filesize: %ld bytes\n", fileSize);
  fread(gameStart, 1, (fileSize / 8), file);
  printf("Read file successfully!\n");

  // Read the game header and initialize CPU accordingly
  readGameHeaders(cpu);
  execute(cpu);
  fclose(file);
}
