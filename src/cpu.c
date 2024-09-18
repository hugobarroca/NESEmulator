// 6502 Processor CPU (based on the 6502 CPU).
#include "cpu.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

void pushStack(CPU *cpu, uint8_t value) {
  if (cpu->S < 0x00) {
    printf("ERROR; Stack overflow detected!\n");
  }
  cpu->Memory[cpu->S + 0x0100] = value;
  cpu->S--;
}

uint8_t popStack(CPU *cpu) {
  if (cpu->S == 0xFF) {
    printf("ERROR: Stack underflow detected!\n");
  }
  cpu->S++;
  return cpu->Memory[(cpu->S - 1) + 0x0100];
}

uint8_t readBusMapperZero(CPU *cpu, uint16_t address) {
  // For NROM Mapper
  // CPU $6000-$7FFF
  // CPU $8000-$BFFF First 16KB of ROM
  // CPU $C000-$FFFF Last 16KB of ROM

  // Get it from zero-page memory.
  if (address >= 0x00 && address <= 0x00FF) {
  }

  // Get it from the stack.
  if (address >= 0x0100 && address <= 0x01FF) {
    return popStack(cpu);
  }

  if (address >= 0x8000 && address <= 0xBFFF) {
    // Subtract 0xC000 for mapping, and add 0x0010 for header
    uint16_t gameAddress = address - 0xBFF0;
    return cpu->GameData[gameAddress];
  }

  if (address >= 0xC000 && address <= 0xFFFF) {
    // printf("Address value: 0x%02x\n", address);
    uint16_t gameAddress = address - 0xBFF0;
    // printf("Game address value: 0x%02x\n", gameAddress);
    return cpu->GameData[gameAddress];
  }

  printf("ERROR: Invalid bus address was accessed!\n");
  return -1;
}

uint8_t readBus(CPU *cpu, uint16_t address) {
  return cpu->ReadBus(cpu, address);
}

void setAndPrintMapper(CPU *cpu, uint8_t mapperNumber) {
  switch (mapperNumber) {
  case 0:
    printf("NROM Mapper recognized!\n");
    cpu->ReadBus = readBusMapperZero;
    break;
  case 4:
    printf("Nintendo MMC3 Mapper recognized!\n");
    break;
  default:
    printf("Unrecognized mapper.\n");
  }
}

// Fetches instruction from memory at PC location, and increments PC
uint8_t fetchInstructionByte(CPU *cpu) {
  uint8_t instruction = readBus(cpu, cpu->PC);
  printf("\n Fetched: 0x%02x ", instruction);
  cpu->PC++;
  return instruction;
}

uint8_t fetchImmediate(CPU *cpu) { return fetchInstructionByte(cpu); }

uint16_t fetchAbsoluteAddress(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  return (uint16_t)hh << 8 | ll;
}

uint8_t fetchAbsolute(CPU *cpu) {
  uint16_t address = fetchAbsoluteAddress(cpu);
  return readBus(cpu, address);
}

uint16_t fetchZeroPageAddress(CPU *cpu) {
  uint8_t result = readBus(cpu, cpu->PC);
  return result;
}

uint8_t fetchZeroPage(CPU *cpu) {
  // Instruction is a ZeroPage memory address
  uint16_t instruction = fetchZeroPageAddress(cpu);
  return readBus(cpu, instruction);
}

uint8_t fetchAbsoluteX(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t address = (uint16_t)hh << 8 | ll;
  uint16_t effectiveAddress = address + cpu->X;
  return readBus(cpu, address);
}

uint8_t fetchAbsoluteY(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t address = (uint16_t)hh << 8 | ll;
  uint16_t effectiveAddress = address + cpu->Y;
  return readBus(cpu, address);
}

uint8_t fetchZeroPageX(CPU *cpu) {
  uint8_t instruction = readBus(cpu, cpu->PC);
  uint16_t effectiveAddress = instruction + cpu->X;
  return readBus(cpu, (uint16_t)instruction);
}

uint8_t fetchZeroPageY(CPU *cpu) {
  uint8_t instruction = readBus(cpu, cpu->PC);
  uint16_t effectiveAddress = instruction + cpu->X;
  return readBus(cpu, (uint16_t)instruction);
}

uint16_t fetchIndirectAddress(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t instructionAddress = (uint16_t)hh << 8 | ll;
  uint8_t llIndirect = readBus(cpu, instructionAddress);
  uint8_t hhIndirect = readBus(cpu, instructionAddress + 1);
  return (uint16_t)hhIndirect << 8 | llIndirect;
}

uint16_t fetchPreIndexedIndirectXAddress(CPU *cpu) {
  uint8_t instructionAddress = fetchInstructionByte(cpu);
  uint8_t indexedAddress = instructionAddress + cpu->X;
  uint8_t ll = readBus(cpu, indexedAddress);
  uint8_t hh = readBus(cpu, indexedAddress + 1);
  return (uint16_t)hh << 8 | ll;
}

uint8_t fetchPreIndexedIndirectX(CPU *cpu) {
  uint16_t effectiveAddress = fetchPreIndexedIndirectXAddress(cpu);
  return readBus(cpu, effectiveAddress);
}

uint16_t fetchPostIndexedIndirectYAddress(CPU *cpu) {
  uint16_t instructionAddress = fetchInstructionByte(cpu);
  uint8_t ll = readBus(cpu, instructionAddress);
  uint8_t hh = readBus(cpu, instructionAddress);
  uint16_t lookupAddress = (uint16_t)hh << 8 | ll;
  return lookupAddress + cpu->Y;
}

uint8_t fetchPostIndexedIndirectY(CPU *cpu) {
  uint16_t effectiveAddress = fetchPostIndexedIndirectYAddress(cpu);
  return readBus(cpu, effectiveAddress);
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

// Sets the overflow flag to 1 if original is smaller than the result
void setOverflowFlagIfOverflow(CPU *cpu, uint8_t larger, uint8_t smaller) {
  if (smaller > larger) {
    cpu->P = cpu->P | 0x40;
  }
}

void setCarryFlagConditionally(CPU *cpu, bool condition) {
  if (condition) {
    cpu->P = cpu->P | 0x01;
  } else {
    cpu->P = cpu->P & 0xFE;
  }
}

uint8_t isCarryFlagSet(CPU *cpu) { return (cpu->P & 0x01) == 0x01; }

// ------------- INSTRUCTIONS -------------

// ADC Add Memory to Accumulator with Carry
// NZCIDV
// +++--+

void addWithCarry(CPU *cpu, uint8_t oper) {
  bool hasCarry = isCarryFlagSet(cpu);
  uint8_t result = cpu->A + oper + (uint8_t)hasCarry;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  setOverflowFlagIfOverflow(cpu, cpu->A, result);
  setCarryFlagConditionally(cpu, cpu->A > result);
  cpu->A = result;
}

// 0x69, ADC #oper, 2 bytes, 2 cycles
void addWithCarryImmediate(CPU *cpu) {
  uint8_t oper = fetchImmediate(cpu);
  addWithCarry(cpu, oper);
}

// 0x65, ADC oper, 2 bytes, 2 cycles
void addWithCarryZeroPage(CPU *cpu) {
  uint8_t oper = fetchZeroPage(cpu);
  addWithCarry(cpu, oper);
}

// 0x75, ADC oper,X, 2 bytes, 4 cycles
void addWithCarryZeroPageX(CPU *cpu) {
  uint8_t oper = fetchZeroPageX(cpu);
  addWithCarry(cpu, oper);
}

// 0x6D, ADC oper,X, 2 bytes, 4 cycles
void addWithCarryAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  addWithCarry(cpu, oper);
}

// 0x7D, ADC oper, 3 bytes, 4* cycles
void addWithCarryAbsoluteX(CPU *cpu) {
  uint8_t oper = fetchAbsoluteX(cpu);
  addWithCarry(cpu, oper);
}

// 0x79, ADC oper,Y, 3 bytes, 4* cycles
void addWithCarryAbsoluteY(CPU *cpu) {
  uint8_t oper = fetchAbsoluteY(cpu);
  addWithCarry(cpu, oper);
}

// 0x61, ADC (oper,X), 2 bytes, 6 cycles
void addWithCarryIndirectX(CPU *cpu) {
  uint8_t oper = fetchPreIndexedIndirectX(cpu);
  addWithCarry(cpu, oper);
}

// 0x71, ADC (oper),Y, 2 bytes, 5* cycles
void addWithCarryIndirectY(CPU *cpu) {
  uint8_t oper = fetchPostIndexedIndirectY(cpu);
  addWithCarry(cpu, oper);
}

// AND Memory with Accumulator
// NZCIDV
// ++----
void andMemoryWithAccumulator(CPU *cpu, uint8_t oper) {
  uint8_t result = cpu->A | oper;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x29, AND #oper, 2 bytes, 2 cycles
void andImmediate(CPU *cpu) {
  uint8_t oper = fetchPreIndexedIndirectX(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

// 0x25
void andZeroPage(CPU *cpu) {
  uint8_t oper = fetchZeroPage(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

// 0x35
void andZeroPageX(CPU *cpu) {
  uint8_t oper = fetchZeroPageX(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

// 0x2D
void andAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

// 0x3D
void andAbsoluteX(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteX(cpu);
  andMemoryWithAccumulator(cpu, memValue);
}

// 0x39
void andAbsoluteY(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteY(cpu);
  andMemoryWithAccumulator(cpu, memValue);
}

// 0x21, AND (oper, X), 2 bytes, 6 cycles
void andIndirectX(CPU *cpu) {
  uint8_t oper = fetchPreIndexedIndirectX(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

void andIndirectY(CPU *cpu) {
  uint8_t oper = fetchPostIndexedIndirectY(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

// ASL Shift Left One Bit (Memory or Accumulator
//
void arithmeticShiftLeft(CPU *cpu, uint8_t oper) {
  uint8_t result = oper << 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  setOverflowFlagIfOverflow(cpu, oper, result);
  cpu->Memory[oper] = result;
}

// 0x0A, ASL, NZC, 1 byte, 2 cycles
void arithmeticShiftLeftAccumulator(CPU *cpu) {
  setCarryFlagConditionally(cpu, (cpu->A & 0x80) == 0x80);
  uint8_t result = cpu->A << 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x06, ASL oper, NZC, 2 bytes, 5 cycles
void arithmeticShiftLeftZeroPage(CPU *cpu) {
  uint8_t oper = fetchInstructionByte(cpu);
  arithmeticShiftLeft(cpu, oper);
}

// 0x16, ASL oper,X, NZC, 2 bytes, 6 cycles
void arithmeticShiftLeftZeroPageX(CPU *cpu) {
  uint8_t oper = fetchZeroPageX(cpu);
  arithmeticShiftLeft(cpu, oper);
}

// 0x0E, ASL oper, NZC, 3 bytes, 6 cycles
void arithmeticShiftLeftAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  arithmeticShiftLeft(cpu, oper);
}

// 0x1E, ASL oper,X, NZC, 3 bytes, 7 cycles
void arithmeticShiftLeftAbsoluteX(CPU *cpu) {
  uint8_t oper = fetchAbsoluteX(cpu);
  arithmeticShiftLeft(cpu, oper);
}

// BCC Branch on Carry Clear
void branchOnClearCarryRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x01) == 0x00) {
    cpu->P = cpu->P + address;
  }
}

// BCS Branch on Carry Set
void branchOnCarrySetRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x01) == 0x01) {
    cpu->P = cpu->P + address;
  }
}

// BEQ Branch on Result Zero
void branchOnEqualRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x02) == 0x02) {
    cpu->P = cpu->P + address;
  }
}

// BIT Test

// 0x24, BIT oper, Z, 2 bytes, 3 cycles
// Addressing Mode: ZeroPage
void bitTestZeroPage(CPU *cpu) {
  uint8_t memValue = fetchZeroPage(cpu);
  uint8_t nvBits = memValue | 0x3F; // Mask out irrelevant bits
  uint8_t result = cpu->P | 0xC0 & nvBits;
  setZeroFlagIfZero(cpu, result);
  cpu->P = result;
}

// 0x2C, BIT oper, Z, 3 bytes, 4 cycles
// Addressing Mode: Absolute
void bitTestAbsolute(CPU *cpu) {
  uint8_t memValue = fetchAbsolute(cpu);
  uint8_t nvBits = memValue | 0x3F; // Mask out irrelevant bits
  uint8_t result = cpu->P | 0xC0 & nvBits;
  setZeroFlagIfZero(cpu, result);
  cpu->P = result;
}

// BMI Branch on Result Minus
// 0x30
void branchOnMinusRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x80) == 0x80) {
    cpu->P = cpu->P + address;
  }
}

// BNE Branch on Result not Zero
// 0xD0
void branchOnNotEqualRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x02) == 0x00) {
    cpu->P = cpu->P + address;
  }
}

// BPL Branch on Result Plus
// 0x10, BPL oper, -, 2 bytes, 2 cycles
void branchOnPlusRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x80) == 0x00) {
    cpu->P = cpu->P + address;
  }
}

// 0x00, BRK, I, 1 byte, 7 cycles
void forceBreak(CPU *cpu) {
  // Push PC + 2 to stack;
  pushStack(cpu, cpu->PC + 2);
  // Push Processor Status to stack with I flag set to 1
  pushStack(cpu, cpu->P | 0x04);
  // Sets PC to be equal to FFFE and FFF0
  cpu->PC = cpu->Memory[0xFFFE];
}

// BVC Branch on Overflow Clear
// 0x50
void branchOnOverflowSetRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x40) == 0x00) {
    cpu->P = cpu->P + address;
  }
}

// BVS Branch on Overflow Set
// 0x70
void branchOnOverflowClearRelative(CPU *cpu) {
  uint8_t address = fetchInstructionByte(cpu);
  if ((cpu->P & 0x40) == 0x01) {
    cpu->P = cpu->P + address;
  }
}

// CLC Clear Carry Flag
// 0x18, CLC, -, 1 byte, 2 cycles
void clearCarry(CPU *cpu) { cpu->P = cpu->P & 0xFE; }

// CLD Cleaer Decimal Mode
// 0xD8
void clearDecimal(CPU *cpu) { cpu->P = cpu->P & 0xF7; }

// CLI Clear Interrupt Disable BIt
// 0x58
void clearInterruptDisable(CPU *cpu) { cpu->P = cpu->P & 0xFB; }

// CLV Clear Overflow Flag
// 0xB8
void clearOverflow(CPU *cpu) { cpu->P = cpu->P & 0xBF; }
// CMP Compare Memory with Accumulator
void compareWithAccumulator(CPU *cpu, uint8_t memory) {
  uint8_t result = cpu->A - memory;
  setNegativeFlagIfNegative(cpu, result);
  setZeroFlagIfZero(cpu, result);
  setCarryFlagConditionally(cpu, memory >= cpu->A);
}

// 0xC9
void compareWithAccumulatorImmediate(CPU *cpu) {
  uint8_t oper = fetchImmediate(cpu);
  compareWithAccumulator(cpu, oper);
}

// 0xC5
void compareWithAccumulatorZeroPage(CPU *cpu) {
  uint8_t oper = fetchZeroPage(cpu);
  compareWithAccumulator(cpu, oper);
}

// 0xD5
void compareWithAccumulatorZeroPageX(CPU *cpu) {
  uint8_t oper = fetchZeroPageX(cpu);
  compareWithAccumulator(cpu, oper);
}

// 0xCD
void compareWithAccumulatorAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  compareWithAccumulator(cpu, oper);
}

// 0xDD
void compareWithAccumulatorAbsoluteX(CPU *cpu) {
  uint8_t oper = fetchAbsoluteX(cpu);
  compareWithAccumulator(cpu, oper);
}

// 0xD9
void compareWithAccumulatorAbsoluteY(CPU *cpu) {
  uint8_t oper = fetchAbsoluteY(cpu);
  compareWithAccumulator(cpu, oper);
}

// 0xC1
void compareWithAccumulatorIndirectX(CPU *cpu) {
  uint8_t oper = fetchPreIndexedIndirectX(cpu);
  compareWithAccumulator(cpu, oper);
}

// 0xD1
void compareWithAccumulatorIndirectY(CPU *cpu) {
  uint8_t oper = fetchPostIndexedIndirectY(cpu);
  compareWithAccumulator(cpu, oper);
}

// CPX Compare Memory and Index X
void compareWithX(CPU *cpu, uint8_t memory) {
  uint8_t result = cpu->X - memory;
  setNegativeFlagIfNegative(cpu, result);
  setZeroFlagIfZero(cpu, result);
  setCarryFlagConditionally(cpu, memory >= cpu->X);
}

// 0xE0
void compareWithXImmediate(CPU *cpu) {
  uint8_t oper = fetchImmediate(cpu);
  compareWithX(cpu, oper);
}

// 0xE4
void compareWithXZeroPage(CPU *cpu) {
  uint8_t oper = fetchZeroPage(cpu);
  compareWithX(cpu, oper);
}

// 0xEC
void compareWithXAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  compareWithX(cpu, oper);
}

// CPY Compare Memory and Index Y
void compareWithY(CPU *cpu, uint8_t memory) {
  uint8_t result = cpu->Y - memory;
  setNegativeFlagIfNegative(cpu, result);
  setZeroFlagIfZero(cpu, result);
  setCarryFlagConditionally(cpu, memory >= cpu->Y);
}

// 0xC0
void compareWithYImmediate(CPU *cpu) {
  uint8_t oper = fetchImmediate(cpu);
  compareWithY(cpu, oper);
}

// 0xC4
void compareWithYZeroPage(CPU *cpu) {
  uint8_t oper = fetchZeroPage(cpu);
  compareWithY(cpu, oper);
}

// 0xCC
void compareWithYAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  compareWithY(cpu, oper);
}

// DEC Decrement Memory by One
void decrement(CPU *cpu, uint8_t memAddr) {
  uint8_t result = cpu->Memory[memAddr] - 1;
  setNegativeFlagIfNegative(cpu, result);
  setZeroFlagIfZero(cpu, result);
  cpu->Memory[memAddr] = result;
}

// 0xC6
void decrementZeroPage(CPU *cpu) {
  uint8_t memAddr = fetchZeroPage(cpu);
  decrement(cpu, memAddr);
}

// 0xD6
void decrementZeroPageX(CPU *cpu) {
  uint8_t memAddr = fetchZeroPageX(cpu);
  decrement(cpu, memAddr);
}

// 0xCE
void decrementAbsolute(CPU *cpu) {
  uint8_t memAddr = fetchAbsolute(cpu);
  decrement(cpu, memAddr);
}

// 0xDE
void decrementAbsoluteX(CPU *cpu) {
  uint8_t memAddr = fetchAbsoluteX(cpu);
  decrement(cpu, memAddr);
}

// DEX Decrement Index X by One
// 0xCA
void decrementX(CPU *cpu) {
  uint8_t result = cpu->X - 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->X = result;
}

// DEY Decrement Index Y by One
// 0x88
void decrementY(CPU *cpu) {
  uint8_t result = cpu->Y - 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->Y = result;
}

// EOR Exclusive-Or Memory with Accumulator
void exclusiveOr(CPU *cpu, uint8_t memValue) {
  uint8_t result = cpu->A ^ memValue;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// 0x49
void exclusiveOrImmediate(CPU *cpu) {
  uint8_t oper = fetchImmediate(cpu);
  exclusiveOr(cpu, oper);
}

// 0x45
void exclusiveOrZeroPage(CPU *cpu) {
  uint8_t oper = fetchZeroPage(cpu);
  exclusiveOr(cpu, oper);
}

// 0x55
void exclusiveOrZeroPageX(CPU *cpu) {
  uint8_t oper = fetchZeroPageX(cpu);
  exclusiveOr(cpu, oper);
}

// 0x4D
void exclusiveOrAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  exclusiveOr(cpu, oper);
}

// 0x5D
void exclusiveOrAbsoluteX(CPU *cpu) {
  uint8_t oper = fetchAbsoluteX(cpu);
  exclusiveOr(cpu, oper);
}

// 0x59
void exclusiveOrAbsoluteY(CPU *cpu) {
  uint8_t oper = fetchAbsoluteX(cpu);
  exclusiveOr(cpu, oper);
}

// 0x41
void exclusiveOrIndirectX(CPU *cpu) {
  uint8_t oper = fetchPreIndexedIndirectX(cpu);
  exclusiveOr(cpu, oper);
}

// 0x51
void exclusiveOrIndirectY(CPU *cpu) {
  uint8_t oper = fetchPostIndexedIndirectY(cpu);
  exclusiveOr(cpu, oper);
}

// INC Increment Memory by One
void increment(CPU *cpu, uint8_t memAddr) {
  uint8_t result = cpu->Memory[memAddr] + 1;
  setNegativeFlagIfNegative(cpu, result);
  setZeroFlagIfZero(cpu, result);
  cpu->Memory[memAddr] = result;
}

// 0xE6
void incrementZeroPage(CPU *cpu) {
  uint8_t memAddr = fetchZeroPage(cpu);
  increment(cpu, memAddr);
}

// 0xF6
void incrementZeroPageX(CPU *cpu) {
  uint8_t memAddr = fetchZeroPageX(cpu);
  increment(cpu, memAddr);
}

// 0xEE
void incrementAbsolute(CPU *cpu) {
  uint8_t memAddr = fetchAbsolute(cpu);
  increment(cpu, memAddr);
}

// 0xFE
void incrementAbsoluteX(CPU *cpu) {
  uint8_t memAddr = fetchAbsoluteX(cpu);
  increment(cpu, memAddr);
}

// INX Increment Index X by One
// 0xE8
void incrementX(CPU *cpu) {
  uint8_t result = cpu->X + 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->X = result;
}

// INY Increment Index Y by One
// 0xC8
void incrementY(CPU *cpu) {
  uint8_t result = cpu->Y + 1;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->Y = result;
}

// JMP Jump to new Location
void jump(CPU *cpu, uint16_t address) { cpu->PC = address; }

// 0x4C
void jumpAbsolute(CPU *cpu) {
  uint16_t address = fetchAbsoluteAddress(cpu);
  jump(cpu, address);
}

// 0x6C
void jumpIndirect(CPU *cpu) {
  uint16_t address = fetchIndirectAddress(cpu);
  jump(cpu, address);
}

// JSR Jump to new Location Saving Return Address
// 0x20, JSR oper, -, 3 bytes, 6 cycles
void jumpSubRoutineAbsolute(CPU *cpu) {
  pushStack(cpu, cpu->PC + 2);
  uint16_t address = fetchAbsoluteAddress(cpu);
  cpu->PC = address;
}

// LDA Load Accumulator with Memory
void loadAccumulator(CPU *cpu, uint8_t value) {
  setZeroFlagIfZero(cpu, value);
  setNegativeFlagIfNegative(cpu, value);
  cpu->A = value;
}

// 0xA9
void loadAccumulatorImmediate(CPU *cpu) {
  uint8_t value = fetchImmediate(cpu);
  loadAccumulator(cpu, value);
}

// 0xA5
void loadAccumulatorZeroPage(CPU *cpu) {
  uint8_t value = fetchZeroPage(cpu);
  loadAccumulator(cpu, value);
}

// 0xB5
void loadAccumulatorZeroPageX(CPU *cpu) {
  uint8_t value = fetchZeroPageX(cpu);
  loadAccumulator(cpu, value);
}

// 0xAD
void loadAccumulatorAbsolute(CPU *cpu) {
  uint8_t value = fetchAbsolute(cpu);
  loadAccumulator(cpu, value);
}

// 0xBD
void loadAccumulatorAbsoluteX(CPU *cpu) {
  uint8_t value = fetchAbsoluteX(cpu);
  loadAccumulator(cpu, value);
}

// 0xB9
void loadAccumulatorAbsoluteY(CPU *cpu) {
  uint8_t value = fetchAbsoluteY(cpu);
  loadAccumulator(cpu, value);
}

// 0xA1
void loadAccumulatorIndirectX(CPU *cpu) {
  uint8_t value = fetchPreIndexedIndirectX(cpu);
  loadAccumulator(cpu, value);
}

// 0xB1
void loadAccumulatorIndirectY(CPU *cpu) {
  uint8_t value = fetchPostIndexedIndirectY(cpu);
  loadAccumulator(cpu, value);
}

// LDX Load Index X with Memory
void loadX(CPU *cpu, uint8_t value) {
  setZeroFlagIfZero(cpu, value);
  setNegativeFlagIfNegative(cpu, value);
  cpu->X = value;
}

// 0xA2
void loadXImmediate(CPU *cpu) {
  uint8_t value = fetchImmediate(cpu);
  loadX(cpu, value);
}

// 0xA6
void loadXZeroPage(CPU *cpu) {
  uint8_t value = fetchZeroPage(cpu);
  loadX(cpu, value);
}

// 0xB6
void loadXZeroPageY(CPU *cpu) {
  uint8_t value = fetchZeroPageY(cpu);
  loadX(cpu, value);
}

// 0xAE
void loadXAbsolute(CPU *cpu) {
  uint8_t value = fetchAbsolute(cpu);
  loadX(cpu, value);
}

// 0xBE
void loadXAbsoluteY(CPU *cpu) {
  uint8_t value = fetchAbsoluteY(cpu);
  loadX(cpu, value);
}

// LDY Load Index Y with Memory
void loadY(CPU *cpu, uint8_t value) {
  setZeroFlagIfZero(cpu, value);
  setNegativeFlagIfNegative(cpu, value);
  cpu->Y = value;
}

// 0xA0
void loadYImmediate(CPU *cpu) {
  uint8_t value = fetchImmediate(cpu);
  loadY(cpu, value);
}

// 0xA4
void loadYZeroPage(CPU *cpu) {
  uint8_t value = fetchZeroPage(cpu);
  loadY(cpu, value);
}

// 0xB4
void loadYZeroPageX(CPU *cpu) {
  uint8_t value = fetchZeroPageX(cpu);
  loadY(cpu, value);
}

// 0xAC
void loadYAbsolute(CPU *cpu) {
  uint8_t value = fetchAbsolute(cpu);
  loadY(cpu, value);
}

// 0xBC
void loadYAbsoluteX(CPU *cpu) {
  uint8_t value = fetchAbsoluteX(cpu);
  loadY(cpu, value);
}

// LSR Shift One Bit Right (Memory or Accumulator)
uint8_t logisticalShiftRight(CPU *cpu, uint8_t value) {
  uint8_t result = value >> 1;
  // Reset N flag
  cpu->P = cpu->P & 0x7F;
  setZeroFlagIfZero(cpu, result);
  // Set 0th bit to Carry
  uint8_t zeroBit = value & 0x01;
  if (zeroBit == 0x01) {
    cpu->P = cpu->P | 0x01;
  } else {
    cpu->P = cpu->P & 0xFE;
  }
  return result;
}

// 0x4A
void logisticalShiftRightAccumulator(CPU *cpu) {
  uint8_t result = logisticalShiftRight(cpu, cpu->A);
  cpu->A = result;
}

// 0x46
void logisticalShiftRightZeroPage(CPU *cpu) {
  uint16_t address = fetchZeroPageAddress(cpu);
  uint8_t value = readBus(cpu, address);
  uint8_t result = logisticalShiftRight(cpu, value);
  cpu->Memory[address] = result;
}

// 0x56
void logisticalShiftRightZeroPageX(CPU *cpu) {
  uint16_t address = fetchZeroPageAddress(cpu) + cpu->X;
  uint8_t value = readBus(cpu, address);
  uint8_t result = logisticalShiftRight(cpu, value);
  cpu->Memory[address] = result;
}

// 0x4E
void logisticalShiftRightAbsolute(CPU *cpu) {
  uint16_t address = fetchAbsoluteAddress(cpu);
  uint8_t value = readBus(cpu, address);
  uint8_t result = logisticalShiftRight(cpu, value);
  cpu->Memory[address] = result;
}

// 0x5E
void logisticalShiftRightAbsoluteX(CPU *cpu) {
  uint16_t address = fetchAbsoluteAddress(cpu) + cpu->X;
  uint8_t value = readBus(cpu, address);
  uint8_t result = logisticalShiftRight(cpu, value);
  cpu->Memory[address] = result;
}

// NOP No Operation
// 0xEA
void noOperation(CPU *cpu) { return; }

// ORA Or Memory with Accumulator
uint8_t orA(CPU *cpu, uint8_t memValue) {
  uint8_t result = memValue | cpu->A;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  return result;
}

// 0x09, ORA #oper, NZ, 2 bytes, 2 cycles
void orAImmediate(CPU *cpu) {
  uint8_t memValue = fetchImmediate(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// 0x05, ORA oper, NZ, 2 bytes, 3 cycles
void orAZeroPage(CPU *cpu) {
  uint8_t memValue = fetchZeroPage(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// 0x15, ORA oper,X, NZ, 2 bytes, 4 cycles
void orAZeroPageX(CPU *cpu) {
  uint8_t memValue = fetchZeroPageX(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// 0x0D, ORA oper, NZ, 3 bytes, 4 cycles
void orAAbsolute(CPU *cpu) {
  uint8_t memValue = fetchAbsolute(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// 0x1D, ORA oper,X, NZ, 3 bytes, 4 cycles
void orAAbsoluteX(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteX(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// 0x19, ORA oper,Y, NZ, 3 bytes, 4* cycles
void orAAbsoluteY(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteY(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// 0x01, ORA(oper,X), NZ, 2 bytes, 6 cycles
void orAIndirectX(CPU *cpu) {
  uint8_t memValue = fetchPreIndexedIndirectX(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// 0x11, ORA (oper),Y, NZ, 2 bytes, 5 cycles
void orAIndirectY(CPU *cpu) {
  uint8_t memValue = fetchPostIndexedIndirectY(cpu);
  uint8_t result = orA(cpu, memValue);
  cpu->A = result;
}

// PHA Push Accumulator on Stack
// 0x48
void pushAccumulatorOntoStack(CPU *cpu) { pushStack(cpu, cpu->A); }

// PHP Push Processor Status on Status
// 0x08, PHP, -, 1 byte, 3 cycles
void pushProcessorStatusOnStack(CPU *cpu) {
  // Set B Flag and Pin 5 to 1 before pushing
  cpu->P = cpu->P | 0x30; // 00110000
  pushStack(cpu, cpu->P);
}

// PLA Pull Accumulator from Stack
void pullAccumulatorFromStack(CPU *cpu) {
  uint8_t stackValue = popStack(cpu);
  setZeroFlagIfZero(cpu, stackValue);
  setNegativeFlagIfNegative(cpu, stackValue);
  cpu->A = stackValue;
}

// Pull Processor Stauts from Stack
// 0x28
void pullProcessorStatusFromStack(CPU *cpu) { cpu->P = popStack(cpu); }

// ROL Rotate One Bit Left
uint8_t rotateLeft(CPU *cpu, uint8_t value) {
  uint8_t result;
  if ((value | 0x7F) != value) {
    cpu->P = cpu->P | 0x01;
    result = (value << 1) + 1;
  } else {
    cpu->P = cpu->P & 0xFE;
    result = value << 1;
  }
  return result;
}

// 0x2A
void rotateLeftAccumulator(CPU *cpu) {
  uint8_t result = rotateLeft(cpu, cpu->A);
  cpu->A = result;
}

// 0x26
void rotateLeftZeroPage(CPU *cpu) {
  uint8_t memValue = fetchZeroPage(cpu);
  uint8_t result = rotateLeft(cpu, memValue);
  cpu->A = result;
}

// 0x36
void rotateLeftZeroPageX(CPU *cpu) {
  uint8_t memValue = fetchZeroPageX(cpu);
  uint8_t result = rotateLeft(cpu, memValue);
  cpu->A = result;
}

// 0x2E
void rotateLeftAbsolute(CPU *cpu) {
  uint8_t memValue = fetchAbsolute(cpu);
  uint8_t result = rotateLeft(cpu, memValue);
  cpu->A = result;
}

// 0x3E
void rotateLeftAbsoluteX(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteX(cpu);
  uint8_t result = rotateLeft(cpu, memValue);
  cpu->A = result;
}

// ROR Rotate One Bit Right
uint8_t rotateRight(CPU *cpu, uint8_t value) {
  uint8_t result;
  if ((value | 0x01) == value) { // If 0th bit is 1
    cpu->P = cpu->P | 0x01;
    result = (value >> 1) | 0x80;
  } else {
    cpu->P = cpu->P & 0xFE;
    result = value >> 1 & 0x7F;
  }
  return result;
}

// 0x6A
void rotateRightAccumulator(CPU *cpu) {
  uint8_t result = rotateRight(cpu, cpu->A);
  cpu->A = result;
}

// 0x66
void rotateRightZeroPage(CPU *cpu) {
  uint8_t memValue = fetchZeroPage(cpu);
  uint8_t result = rotateRight(cpu, memValue);
  cpu->A = result;
}

// 0x76
void rotateRightZeroPageX(CPU *cpu) {
  uint8_t memValue = fetchZeroPageX(cpu);
  uint8_t result = rotateRight(cpu, memValue);
  cpu->A = result;
}

// 0x6E
void rotateRightAbsolute(CPU *cpu) {
  uint8_t memValue = fetchAbsolute(cpu);
  uint8_t result = rotateRight(cpu, memValue);
  cpu->A = result;
}

// 0x7E
void rotateRightAbsoluteX(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteX(cpu);
  uint8_t result = rotateRight(cpu, memValue);
  cpu->A = result;
}

// RTI Return from Interrupt
// 0x40
void returnFromInterrupt(CPU *cpu) {
  cpu->P = popStack(cpu);
  uint8_t pcl = popStack(cpu);
  uint8_t pch = popStack(cpu);
  cpu->PC = (pch << 8) + pcl;
}

// RTS Return from Subroutine
// 0x60
void returnFromSubroutine(CPU *cpu) {}

// SBC Subtract Memory from Accumulator with Borrow
uint8_t subtractWithCarry(CPU *cpu, uint8_t value) {
  uint8_t result;
  if (cpu->A > value) {
    // No borrow
    cpu->P = cpu->P | 0x01;
  } else {
    cpu->P = cpu->P & 0xFE;
  }
  result = cpu->A - value;
  // Set overflow
  setOverflowFlagIfOverflow(cpu, cpu->A, result);
  setNegativeFlagIfNegative(cpu, result);
  return result;
}

// 0xE9
void subtractWithCarryImmediate(CPU *cpu) {
  uint8_t memValue = fetchImmediate(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// 0xE5
void subtractWithCarryZeroPage(CPU *cpu) {
  uint8_t memValue = fetchZeroPage(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// 0xF5
void subtractWithCarryZeroPageX(CPU *cpu) {
  uint8_t memValue = fetchZeroPageX(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// 0xED
void subtractWithCarryAbsolute(CPU *cpu) {
  uint8_t memValue = fetchAbsolute(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// 0xFD
void subtractWithCarryAbsoluteX(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteX(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// 0xF9
void subtractWithCarryAbsoluteY(CPU *cpu) {
  uint8_t memValue = fetchAbsoluteY(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// 0xE1
void subtractWithCarryIndirectX(CPU *cpu) {
  uint8_t memValue = fetchPreIndexedIndirectX(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// 0xF1
void subtractWithCarryIndirectY(CPU *cpu) {
  uint8_t memValue = fetchPostIndexedIndirectY(cpu);
  uint8_t result = subtractWithCarry(cpu, memValue);
}

// SEC Set Carry Flag
// 0x38
void setCarry(CPU *cpu) { cpu->P = cpu->P | 0x01; }

// SED Set Decimal Flag
// 0xF8
void setDecimal(CPU *cpu) { cpu->P = cpu->P | 0x08; }

// SEI Set Interrupt Disable Status
// 0x78
void setInterruptDisable(CPU *cpu) { cpu->P = cpu->P | 0x04; }

// STA Store Accumulator in Memory
// 0x85
void storeAccumulatorZeroPage(CPU *cpu) {
  uint16_t memAddr = fetchZeroPageAddress(cpu);
  cpu->Memory[memAddr] = cpu->A;
}

// 0x95
void storeAccumulatorZeroPageX(CPU *cpu) {
  uint16_t memAddr = fetchZeroPageAddress(cpu) + cpu->X;
  cpu->Memory[memAddr] = cpu->A;
}

// 0x8D
void storeAccumulatorAbsolute(CPU *cpu) {
  uint16_t memAddr = fetchAbsoluteAddress(cpu);
  cpu->Memory[memAddr] = cpu->A;
}

// 0x9D
void storeAccumulatorAbsoluteX(CPU *cpu) {
  uint16_t memAddr = fetchAbsoluteAddress(cpu) + cpu->X;
  cpu->Memory[memAddr] = cpu->A;
}

// 0x99
void storeAccumulatorAbsoluteY(CPU *cpu) {
  uint16_t memAddr = fetchAbsoluteAddress(cpu) + cpu->Y;
  cpu->Memory[memAddr] = cpu->A;
}

// 0x81
void storeAccumulatorIndirectX(CPU *cpu) {
  uint16_t memAddr = fetchPreIndexedIndirectXAddress(cpu) + cpu->Y;
  cpu->Memory[memAddr] = cpu->A;
}

// 0x91
void storeAccumulatorIndirectY(CPU *cpu) {
  uint16_t memAddr = fetchPostIndexedIndirectYAddress(cpu);
  cpu->Memory[memAddr] = cpu->A;
}

// STX Store Index X in Memory
// 0x86
void storeXZeroPage(CPU *cpu) {
  uint16_t memAddr = fetchZeroPageAddress(cpu);
  cpu->Memory[memAddr] = cpu->X;
}

// 0x96
void storeXZeroPageY(CPU *cpu) {
  uint16_t memAddr = fetchZeroPageAddress(cpu) + cpu->Y;
  cpu->Memory[memAddr] = cpu->X;
}

// 0x8E
void storeXAbsolute(CPU *cpu) {
  uint16_t memAddr = fetchAbsoluteAddress(cpu);
  cpu->Memory[memAddr] = cpu->X;
}

// STX Store Index Y in Memory
// 0x84
void storeYZeroPage(CPU *cpu) {
  uint16_t memAddr = fetchZeroPageAddress(cpu);
  cpu->Memory[memAddr] = cpu->Y;
}

// 0x94
void storeYZeroPageX(CPU *cpu) {
  uint16_t memAddr = fetchZeroPageAddress(cpu) + cpu->X;
  cpu->Memory[memAddr] = cpu->Y;
}

// 0x8C
void storeYAbsolute(CPU *cpu) {
  uint16_t memAddr = fetchAbsoluteAddress(cpu);
  cpu->Memory[memAddr] = cpu->Y;
}

// TAX Transfer Accumulator to Index X
// 0xAA
void transferAccumulatorToX(CPU *cpu) {
  uint8_t result = cpu->X;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->X = result;
}

// TAY Transfer Accumulator to Index Y
// 0xA8
void transferAccumulatorToY(CPU *cpu) {
  uint8_t result = cpu->Y;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->Y = result;
}

// TSX Transfer Stack Pointer to Index X
// 0xBA
void transferStackPointerToX(CPU *cpu) {
  uint8_t result = cpu->S;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->X = result;
}

// TXA Transfer Index X to Accumulator
// 0x8A
void transferXToAccumulator(CPU *cpu) {
  uint8_t result = cpu->X;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

// TXS Trasnfer Index X to Stack Pointer
// 0x9A
void transferXToStackPointer(CPU *cpu) {
  uint8_t result = cpu->X;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->S = result;
}

// TYA Transfer Index Y to Accumulator
// 0x98
void transferYToAccumulator(CPU *cpu) {
  uint8_t result = cpu->Y;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

void executeInstruction(CPU *cpu) {
  uint8_t instruction = readBus(cpu, cpu->PC);
  printf("PC: 0x%04x ", cpu->PC);
  printf("NESMEM: 0x%04x ", cpu->PC - 0xBFF0);
  printf("Instruction: 0x%02x ", instruction);
  printf("N:%d ", (cpu->P & 0x80) >> 7);
  printf("V:%d ", (cpu->P & 0x40) >> 6);
  printf("U:%d ", (cpu->P & 0x20) >> 5);
  printf("B:%d ", (cpu->P & 0x10) >> 4);
  printf("D:%d ", (cpu->P & 0x08) >> 3);
  printf("I:%d ", (cpu->P & 0x04) >> 2);
  printf("Z:%d ", (cpu->P & 0x02) >> 1);
  printf("C:%d ", cpu->P & 0x01);
  cpu->PC++;
  switch (instruction) {
  case (0x00):
    printf("BRK");
    forceBreak(cpu);
    break;
  case (0x01):
    printf("ORA (oper,X)");
    orAIndirectX(cpu);
    break;
  case (0x05):
    printf("ORA oper");
    orAZeroPage(cpu);
    break;
  case (0x06):
    printf("ASL oper");
    arithmeticShiftLeftZeroPage(cpu);
    break;
  case (0x08):
    printf("PHP");
    pushProcessorStatusOnStack(cpu);
    break;
  case (0x09):
    printf("ORA #oper");
    orAImmediate(cpu);
    break;
  case (0x0A):
    printf("ASL");
    arithmeticShiftLeftAccumulator(cpu);
    break;
  case (0x0D):
    printf("ORA oper");
    orAAbsolute(cpu);
    break;
  case (0x0E):
    printf("ASL opr");
    arithmeticShiftLeftAbsolute(cpu);
    break;
  case (0x10):
    printf("BPL oper");
    branchOnPlusRelative(cpu);
    break;
  case (0x11):
    printf("ORA (oper),Y");
    orAIndirectY(cpu);
    break;
  case (0x15):
    printf("ORA oper,X");
    orAZeroPageX(cpu);
    break;
  case (0x16):
    printf("ASL oper,X");
    arithmeticShiftLeftZeroPageX(cpu);
    break;
  case (0x18):
    printf("CLC");
    clearCarry(cpu);
    break;
  case (0x19):
    printf("ORA oper,Y");
    orAAbsoluteY(cpu);
    break;
  case (0x1D):
    printf("ORA oper,X");
    orAAbsoluteX(cpu);
    break;
  case (0x1E):
    printf("ASL oper,X");
    arithmeticShiftLeftAbsoluteX(cpu);
    break;
  case (0x20):
    printf("JSR");
    jumpSubRoutineAbsolute(cpu);
    break;
  case (0x21):
    printf("AND (oper,X)");
    andIndirectX(cpu);
    break;
  case (0x24):
    printf("BIT oper");
    bitTestZeroPage(cpu);
    break;
  case (0x25):
    printf("AND oper");
    andZeroPage(cpu);
    break;
  case (0x26):
    printf("ROL oper");
    rotateLeftZeroPage(cpu);
    break;
  case (0x28):
    printf("PLP");
    pullProcessorStatusFromStack(cpu);
    break;
  case (0x29):
    printf("AND #oper");
    andImmediate(cpu);
    break;
  case (0x2A):
    printf("ROL A");
    rotateLeftAccumulator(cpu);
    break;
  case (0x2C):
    printf("BIT oper");
    bitTestAbsolute(cpu);
    break;
  case (0x2D):
    printf("AND oper");
    andAbsolute(cpu);
    break;
  case (0x2E):
    printf("ROL oper");
    rotateLeftAbsolute(cpu);
    break;
  case (0x30):
    printf("BMI");
    branchOnMinusRelative(cpu);
    break;
  case (0x31):
    printf("AND (oper),Y");
    andIndirectY(cpu);
    break;
  case (0x35):
    printf("AND oper,X");
    andZeroPageX(cpu);
    break;
  case (0x36):
    printf("ROL oper,X");
    rotateLeftZeroPageX(cpu);
    break;
  case (0x38):
    printf("SEC");
    setCarry(cpu);
    break;
  case (0x39):
    printf("AND oper,Y");
    andAbsoluteY(cpu);
    break;
  case (0x3D):
    printf("AND oper,X");
    andAbsoluteX(cpu);
    break;
  case (0x3E):
    printf("ROL oper,X");
    rotateLeftAbsoluteX(cpu);
    break;
  case (0x40):
    printf("RTI");
    returnFromInterrupt(cpu);
    break;
  case (0x41):
    printf("EOR (oper,X)");
    exclusiveOrIndirectX(cpu);
    break;
  case (0x45):
    printf("EOR oper");
    exclusiveOrZeroPage(cpu);
    break;
  case (0x46):
    printf("LSR oper");
    logisticalShiftRightZeroPage(cpu);
    break;
  case (0x48):
    printf("PHA");
    pushAccumulatorOntoStack(cpu);
    break;
  case (0x49):
    printf("EOR #oper");
    exclusiveOrImmediate(cpu);
    break;
  case (0x4A):
    printf("LSR A");
    logisticalShiftRightAccumulator(cpu);
    break;
  case (0x4C):
    printf("JMP");
    jumpAbsolute(cpu);
    break;
  case (0x4D):
    printf("EOR oper");
    exclusiveOrAbsolute(cpu);
    break;
  case (0x4E):
    printf("LSR oper");
    logisticalShiftRightAbsolute(cpu);
    break;
  case (0x50):
    printf("BVC oper");
    branchOnOverflowClearRelative(cpu);
    break;
  case (0x51):
    printf("EOR (oper),Y");
    exclusiveOrIndirectY(cpu);
    break;
  case (0x55):
    printf("EOR oper,X");
    exclusiveOrZeroPageX(cpu);
    break;
  case (0x56):
    printf("LSR oper,X");
    logisticalShiftRightZeroPageX(cpu);
    break;
  case (0x58):
    printf("CLI");
    clearInterruptDisable(cpu);
    break;
  case (0x59):
    printf("EOR oper,Y");
    exclusiveOrAbsoluteY(cpu);
    break;
  case (0x5D):
    printf("EOR oper,X");
    exclusiveOrAbsoluteX(cpu);
    break;
  case (0x5E):
    printf("LSR oper,X");
    logisticalShiftRightAbsoluteX(cpu);
    break;
  case (0x60):
    printf("RTS");
    returnFromSubroutine(cpu);
    break;
  case (0x61):
    printf("ADC (oper,X)");
    addWithCarryIndirectX(cpu);
    break;
  case (0x65):
    printf("ADC oper");
    addWithCarryZeroPage(cpu);
    break;
  case (0x66):
    printf("ROR oper");
    rotateRightZeroPage(cpu);
    break;
  case (0x68):
    printf("PLA");
    pullAccumulatorFromStack(cpu);
    break;
  case (0x69):
    printf("ADC #oper");
    addWithCarryImmediate(cpu);
    break;
  case (0x6A):
    printf("ROR A");
    rotateRightAccumulator(cpu);
    break;
  case (0x6C):
    printf("JMP (oper)");
    jumpIndirect(cpu);
    break;
  case (0x6D):
    printf("ADC oper");
    addWithCarryAbsolute(cpu);
    break;
  case (0x6E):
    printf("ROR oper");
    rotateRightAbsolute(cpu);
    break;
  case (0x70):
    printf("BVS");
    branchOnOverflowSetRelative(cpu);
    break;
  case (0x71):
    printf("ADC (oper),Y");
    addWithCarryIndirectY(cpu);
    break;
  case (0x75):
    printf("ADC oper,X");
    addWithCarryZeroPageX(cpu);
    break;
  case (0x76):
    printf("ROR oper,X");
    rotateRightZeroPageX(cpu);
    break;
  case (0x78):
    printf("SEI");
    setInterruptDisable(cpu);
    break;
  case (0x79):
    printf("ADC oper,Y");
    addWithCarryAbsoluteY(cpu);
    break;
  case (0x7D):
    printf("ADC oper,X");
    addWithCarryAbsoluteX(cpu);
    break;
  case (0x7E):
    printf("ROR oper,X");
    rotateRightAbsoluteX(cpu);
    break;
  case (0x81):
    printf("STA (oper,X)");
    storeAccumulatorIndirectX(cpu);
    break;
  case (0x84):
    printf("STY oper");
    storeYZeroPage(cpu);
    break;
  case (0x85):
    printf("STA oper");
    storeAccumulatorZeroPage(cpu);
    break;
  case (0x86):
    printf("STX oper");
    storeXZeroPage(cpu);
    break;
  case (0x88):
    printf("DEY");
    decrementY(cpu);
    break;
  case (0x8A):
    printf("TXA");
    transferXToAccumulator(cpu);
    break;
  case (0x8C):
    printf("STY oper");
    storeYAbsolute(cpu);
    break;
  case (0x8D):
    printf("STA oper");
    storeAccumulatorAbsolute(cpu);
    break;
  case (0x8E):
    printf("STX oper");
    storeXAbsolute(cpu);
    break;
  case (0x90):
    printf("BCC oper");
    branchOnClearCarryRelative(cpu);
    break;
  case (0x91):
    printf("STA (oper),Y");
    storeAccumulatorIndirectY(cpu);
    break;
  case (0x94):
    printf("STY oper,X");
    storeYZeroPageX(cpu);
    break;
  case (0x95):
    printf("STA oper,X");
    storeAccumulatorZeroPageX(cpu);
    break;
  case (0x96):
    printf("STX oper,Y");
    storeXZeroPageY(cpu);
    break;
  case (0x98):
    printf("TYA");
    transferYToAccumulator(cpu);
    break;
  case (0x99):
    printf("STA oper,Y");
    storeAccumulatorAbsoluteY(cpu);
    break;
  case (0x9A):
    printf("TXS");
    transferXToStackPointer(cpu);
    break;
  case (0x9D):
    printf("STA oper,X");
    storeAccumulatorAbsoluteX(cpu);
    break;
  case (0xA0):
    printf("LDY #oper");
    loadYImmediate(cpu);
    break;
  case (0xA1):
    printf("LDA (oper,X)");
    loadAccumulatorIndirectX(cpu);
    break;
  case (0xA2):
    printf("LDX #oper");
    loadXImmediate(cpu);
    break;
  case (0xA4):
    printf("LDY oper");
    loadYZeroPage(cpu);
    break;
  case (0xA5):
    printf("LDA oper");
    loadAccumulatorZeroPage(cpu);
    break;
  case (0xA6):
    printf("LDX oper");
    loadXZeroPage(cpu);
    break;
  case (0xA8):
    printf("TAY");
    transferAccumulatorToY(cpu);
    break;
  case (0xA9):
    printf("LDA #oper");
    loadAccumulatorImmediate(cpu);
    break;
  case (0xAA):
    printf("TAX");
    transferAccumulatorToX(cpu);
    break;
  case (0xAC):
    printf("LDY oper");
    loadYAbsolute(cpu);
    break;
  case (0xAD):
    printf("LDA oper");
    loadAccumulatorAbsolute(cpu);
    break;
  case (0xAE):
    printf("LDX oper");
    loadXAbsolute(cpu);
    break;
  case (0xB0):
    printf("BCS oper");
    branchOnCarrySetRelative(cpu);
    break;
  case (0xB1):
    printf("LDA (oper),Y");
    loadAccumulatorIndirectY(cpu);
    break;
  case (0xB4):
    printf("LDY oper,X");
    loadYZeroPageX(cpu);
    break;
  case (0xB5):
    printf("LDA oper,X");
    loadAccumulatorZeroPageX(cpu);
    break;
  case (0xB6):
    printf("LDX oper,Y");
    loadXZeroPageY(cpu);
    break;
  case (0xB8):
    printf("CLV");
    clearOverflow(cpu);
    break;
  case (0xB9):
    printf("LDA oper,Y");
    loadAccumulatorAbsoluteY(cpu);
    break;
  case (0xBA):
    printf("TSX");
    transferStackPointerToX(cpu);
    break;
  case (0xBC):
    printf("LDY oper,X");
    loadYAbsoluteX(cpu);
    break;
  case (0xBD):
    printf("LDA oper,X");
    loadAccumulatorAbsoluteX(cpu);
    break;
  case (0xBE):
    printf("LDX oper,Y");
    loadXAbsoluteY(cpu);
    break;
  case (0xC0):
    printf("CPY #oper");
    compareWithYImmediate(cpu);
    break;
  case (0xC1):
    printf("CMP (oper,X)");
    compareWithAccumulatorIndirectX(cpu);
    break;
  case (0xC4):
    printf("CPY oper");
    compareWithYZeroPage(cpu);
    break;
  case (0xC5):
    printf("CMP oper");
    compareWithAccumulatorZeroPage(cpu);
    break;
  case (0xC6):
    printf("DEC oper");
    decrementZeroPage(cpu);
    break;
  case (0xC8):
    printf("INY");
    incrementY(cpu);
    break;
  case (0xC9):
    printf("CMP #oper");
    compareWithAccumulatorImmediate(cpu);
    break;
  case (0xCA):
    printf("DEX");
    decrementX(cpu);
    break;
  case (0xCC):
    printf("CPY oper");
    compareWithYAbsolute(cpu);
    break;
  case (0xCD):
    printf("CMP oper");
    compareWithAccumulatorAbsolute(cpu);
    break;
  case (0xCE):
    printf("DEC oper");
    decrementAbsolute(cpu);
    break;
  case (0xD0):
    printf("BNE oper");
    branchOnNotEqualRelative(cpu);
    break;
  case (0xD1):
    printf("CMP (oper),Y");
    compareWithAccumulatorIndirectY(cpu);
    break;
  case (0xD5):
    printf("CMP oper,X");
    compareWithAccumulatorZeroPageX(cpu);
    break;
  case (0xD6):
    printf("DEC oper,X");
    decrementZeroPageX(cpu);
    break;
  case (0xD8):
    printf("CLD");
    clearDecimal(cpu);
    break;
  case (0xD9):
    printf("CMP oper,Y");
    compareWithAccumulatorAbsoluteY(cpu);
    break;
  case (0xDD):
    printf("CMP oper,X");
    compareWithAccumulatorAbsoluteX(cpu);
    break;
  case (0xDE):
    printf("DEC oper,X");
    decrementAbsoluteX(cpu);
    break;
  case (0xE0):
    printf("CPX #oper");
    compareWithXImmediate(cpu);
    break;
  case (0xE1):
    printf("SBC (oper,X)");
    subtractWithCarryIndirectX(cpu);
    break;
  case (0xE4):
    printf("CPX oper");
    compareWithXZeroPage(cpu);
    break;
  case (0xE5):
    printf("SBC oper");
    subtractWithCarryZeroPage(cpu);
    break;
  case (0xE6):
    printf("INC oper");
    incrementZeroPage(cpu);
    break;
  case (0xE8):
    printf("INX");
    incrementX(cpu);
    break;
  case (0xE9):
    printf("SBC #oper");
    subtractWithCarryImmediate(cpu);
    break;
  case (0xEA):
    printf("NOP");
    noOperation(cpu);
    break;
  case (0xEC):
    printf("CPX oper");
    compareWithXAbsolute(cpu);
    break;
  case (0xED):
    printf("SBC oper");
    subtractWithCarryAbsolute(cpu);
    break;
  case (0xEE):
    printf("INC oper");
    incrementAbsolute(cpu);
    break;
  case (0xF0):
    printf("BEQ oper");
    branchOnEqualRelative(cpu);
    break;
  case (0xF1):
    printf("SBC (oper),Y");
    subtractWithCarryIndirectY(cpu);
    break;
  case (0xF5):
    printf("SBC oper,X");
    subtractWithCarryZeroPageX(cpu);
    break;
  case (0xF6):
    printf("INC oper,X");
    incrementZeroPageX(cpu);
    break;
  case (0xF8):
    printf("SED");
    setDecimal(cpu);
    break;
  case (0xF9):
    printf("SBC oper,Y");
    subtractWithCarryAbsoluteY(cpu);
    break;
  case (0xFD):
    printf("SBC oper,X");
    subtractWithCarryAbsoluteX(cpu);
    break;
  case (0xFE):
    printf("INC oper,X");
    incrementAbsoluteX(cpu);
    break;
  default:
    printf("UNKNOWN.");
    break;
  }
}

void execute(CPU *cpu) {
  // fetch init vector, and set PT to its value
  uint16_t ll = readBus(cpu, cpu->PC);
  cpu->PC++;
  uint16_t hh = readBus(cpu, cpu->PC);
  uint16_t result = (hh << 8) + ll;
  printf("INIT VECTOR: 0x%02x\n", result);
  cpu->PC = result;
  while (true) {
    executeInstruction(cpu);
    getchar();
  }
}
