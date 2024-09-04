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

// Fetches instruction from memory at PC location, and increments PC
uint8_t fetchInstructionByte(CPU *cpu) {
  uint8_t instruction = readBus(cpu, cpu->PC);
  cpu->PC++;
  return instruction;
}

// Sets the Zero Flag to 1 in the Status Register if result is zero.
void setZeroFlagIfZero(CPU *cpu, uint8_t result) {
  if (result == 0) {
    cpu->P = cpu->P | 0x02;
  }
}

// Sets the Negative Flag to 1 in the Status Register if the result is negative
void setNegativeFlagIfNegative(CPU *cpu, uint8_t result) {
  if ((result & 0x80) == 0x80) {
    cpu->P = cpu->P | 0x02;
  }
}

void setOverflowFlagIfOverflow(CPU *cpu, uint8_t original, uint8_t result) {
  if (result > original) {
    cpu->P = cpu->P | 0x40;
  }
}
// ------------- INSTRUCTIONS -------------

// 0x00, BRK, I, 1 byte, 7 cycles
void forceBreak(CPU *cpu) {
  // Push PC + 2 to stack;
  pushStack(cpu, cpu->PC + 2);
  // Push Processor Status to stack with I flag set to 1
  pushStack(cpu, cpu->P | 0x04);
  // Sets PC to be equal to FFFE and FFF0
  cpu->PC = cpu->Memory[0xFFFE];
}

// Note: This addressing mode is ZeroPage, hence why it only need 2 bytes
// 0x01, ORA(oper,X), NZ, 2 bytes, 6 cycles
void orAIndirectX(CPU *cpu) {
  uint8_t baseAddress = fetchInstructionByte(cpu);
  uint8_t offset = cpu->X;
  // Get second instruction byte
  uint8_t ll = readBus(cpu, baseAddress + cpu->X);
  uint8_t hh = readBus(cpu, baseAddress + cpu->X + 1);
  uint16_t effectiveAddress = (hh << 8) + (uint16_t)ll;
  uint8_t result = cpu->A | readBus(cpu, effectiveAddress);
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x05, ORA oper, NZ, 2 bytes, 3 cycles
void orAZeroPage(CPU *cpu) {
  uint8_t oper = fetchInstructionByte(cpu);
  // An 8-bit addr is enough to access ZeroPage
  uint8_t memValue = readBus(cpu, oper);
  uint8_t result = cpu->A | memValue;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x06, ASL oper, NZC, 2 bytes, 5 cycles
void arithmeticShiftLeftZeroPage(CPU *cpu) {
  uint8_t oper = fetchInstructionByte(cpu);
  uint8_t memValue = readBus(cpu, (uint16_t)oper);
  // Shift value and set it in ZeroPage
  uint8_t result = memValue << 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  setOverflowFlagIfOverflow(cpu, memValue, result);
  cpu->Memory[oper] = result;
}

// 0x08, PHP, -, 1 byte, 3 cycles
void pushProcessorStatusOnStack(CPU *cpu) {
  // Set B Flag and Pin 5 to 1 before pushing
  cpu->P = cpu->P | 0x30; // 00110000
  pushStack(cpu, cpu->P);
}

// 0x09, ORA #oper, NZ, 2 bytes, 2 cycles
void orAImmediate(CPU *cpu) {
  uint8_t oper = fetchInstructionByte(cpu);
  uint8_t result = cpu->A | oper;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x0A, ASL, NZC, 1 byte, 2 cycles
void arithmeticShiftLeftAccumulator(CPU *cpu) {
  // Set Carry Flag
  if ((cpu->A & 0x80) == 0x80) {
    cpu->P = cpu->P | 0x01;
  }
  uint8_t result = cpu->A << 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x0D, ORA oper, NZ, 3 bytes, 4 cycles
void orAAbsolute(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t address = (uint16_t)hh << 8 | ll;
  uint8_t memValue = readBus(cpu, address);
  uint8_t result = cpu->A | memValue;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x0E, ASL oper, NZC, 3 bytes, 6 cycles
void arithmeticShiftLeftAbsolute(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t address = (uint16_t)hh << 8 | ll;
  uint8_t memValue = readBus(cpu, address);
  // Set Carry Flag
  if ((memValue & 0x80) == 0x80) {
    cpu->P = cpu->P | 0x01;
  } else {
    cpu->P = cpu->P & 0xFE;
  }
  uint8_t result = memValue << 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->Memory[address] = result;
}

// 0x15, ORA oper,X, NZ, 2 bytes, 4 cycles
void orAZeroPageX(CPU *cpu) {
  uint8_t oper = fetchInstructionByte(cpu);
  uint16_t address = oper + cpu->X;
  uint8_t memValue = readBus(cpu, address);
  uint8_t result = cpu->A | memValue;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x1D, ORA oper,X, NZ, 3 bytes, 4 cycles
void orAAbsoluteX(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t baseAddress = (uint16_t)hh << 8 | ll;
  uint8_t offset = readBus(cpu, cpu->X);
  // Add it to the memory address value
  uint16_t effectiveAddress = baseAddress + offset;
  uint8_t memValue = readBus(cpu, effectiveAddress);
  uint8_t result = cpu->A | memValue;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x10, BPL oper, -, 2 bytes, 2 cycles
void branchOnPlusRelative(CPU *cpu) {
  uint8_t oper = fetchInstructionByte(cpu);
  if ((cpu->P & 0x80) != 0x80) {
    cpu->PC = cpu->PC + oper;
  }
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
    // ORA (oper,X) ex: ORA ($20,X)
    orAIndirectX(cpu);
    break;
  case (0x05):
    // ORA oper, ex: ORA $20
    orAZeroPage(cpu);
    break;
  case (0x06):
    // ASL oper, ex ASL $20
    arithmeticShiftLeftZeroPage(cpu);
    break;
  case (0x08):
    // PHP
    pushProcessorStatusOnStack(cpu);
    break;
  case (0x09):
    // ORA #oper, ex: ORA #7
    orAImmediate(cpu);
    break;
  case (0x0A):
    // ASL A
    arithmeticShiftLeftAccumulator(cpu);
    break;
  case (0x0D):
    // ORA oper, ex ORA 20
    orAAbsolute(cpu);
    break;
  case (0x0E):
    // ASL opr, ex ASL 20
    arithmeticShiftLeftAbsolute(cpu);
    break;
  case (0x10):
    branchOnPlusRelative(cpu);
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
