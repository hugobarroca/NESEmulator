// This file is meant to hold all the necessary code to emulate the Ricoh 2A03 /
// 6502 Processor CPU (based on the 6502 CPU).
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

uint8_t fetchZeroPage(CPU *cpu) {
  // Instruction is a ZeroPage memory address
  uint8_t instruction = readBus(cpu, cpu->PC);
  return readBus(cpu, (uint16_t)instruction);
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

uint16_t fetchIndirect(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t instructionAddress = (uint16_t)hh << 8 | ll;
  uint8_t llIndirect = readBus(cpu, instructionAddress);
  uint8_t hhIndirect = readBus(cpu, instructionAddress + 1);
  return (uint16_t)hhIndirect << 8 | llIndirect;
}

uint8_t fetchPreIndexedIndirectX(CPU *cpu) {
  uint8_t instructionAddress = fetchInstructionByte(cpu);
  uint8_t indexedAddress = instructionAddress + cpu->X;
  uint8_t ll = readBus(cpu, indexedAddress);
  uint8_t hh = readBus(cpu, indexedAddress + 1);
  uint16_t effectiveAddress = (uint16_t)hh << 8 | ll;
  return readBus(cpu, effectiveAddress);
}

uint8_t fetchPostIndexedIndirectY(CPU *cpu) {
  uint16_t instructionAddress = fetchInstructionByte(cpu);
  uint8_t ll = readBus(cpu, instructionAddress);
  uint8_t hh = readBus(cpu, instructionAddress);
  uint16_t lookupAddress = (uint16_t)hh << 8 | ll;
  uint16_t effectiveAddress = lookupAddress + cpu->Y;
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

void setOverflowFlagIfOverflow(CPU *cpu, uint8_t original, uint8_t result) {
  if (result > original) {
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

// 0x11, ORA (oper),Y, NZ, 2 bytes, 5 cycles
void orAIndirectY(CPU *cpu) {
  uint8_t oper = fetchInstructionByte(cpu);
  uint16_t zpAddress = ((uint16_t)oper << 8) + (oper + 1);
  uint16_t effectiveAddress = zpAddress + cpu->Y;
  uint8_t value = readBus(cpu, effectiveAddress);
  uint8_t result = cpu->A | value;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
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

// 0x19, ORA oper,Y, NZ, 3 bytes, 4* cycles
void orAAbsoluteY(CPU *cpu) {
  uint8_t ll = fetchInstructionByte(cpu);
  uint8_t hh = fetchInstructionByte(cpu);
  uint16_t baseAddress = (uint16_t)hh << 8 | ll;
  uint8_t offset = readBus(cpu, cpu->Y);
  // Add it to the memory address value
  uint16_t effectiveAddress = baseAddress + offset;
  uint8_t memValue = readBus(cpu, effectiveAddress);
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
  uint16_t effectiveAddress = baseAddress + offset;
  uint8_t memValue = readBus(cpu, effectiveAddress);
  uint8_t result = cpu->A | memValue;
  setZeroFlagIfZero(cpu, result);
  setNegativeFlagIfNegative(cpu, result);
  cpu->A = result;
}

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

void andZeroPage(CPU *cpu) {
  uint8_t oper = fetchZeroPage(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

void andZeroPageX(CPU *cpu) {
  uint8_t oper = fetchZeroPageX(cpu);
  andMemoryWithAccumulator(cpu, oper);
}

void andAbsolute(CPU *cpu) {
  uint8_t oper = fetchAbsolute(cpu);
  andMemoryWithAccumulator(cpu, oper);
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
void compareWithXImmediate(CPU *cpu, uint8_t memory) {
  uint8_t oper = fetchImmediate(cpu);
  compareWithX(cpu, oper);
}

// 0xE4
void compareWithXZeroPage(CPU *cpu, uint8_t memory) {
  uint8_t oper = fetchZeroPage(cpu);
  compareWithX(cpu, oper);
}

// 0xEC
void compareWithXAbsolute(CPU *cpu, uint8_t memory) {
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
void compareWithYImmediate(CPU *cpu, uint8_t memory) {
  uint8_t oper = fetchImmediate(cpu);
  compareWithY(cpu, oper);
}

// 0xC4
void compareWithYZeroPage(CPU *cpu, uint8_t memory) {
  uint8_t oper = fetchZeroPage(cpu);
  compareWithY(cpu, oper);
}

// 0xCC
void compareWithYAbsolute(CPU *cpu, uint8_t memory) {
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
  uint16_t address = fetchIndirect(cpu);
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
void loadYZeroPageY(CPU *cpu) {
  uint8_t value = fetchZeroPageY(cpu);
  loadY(cpu, value);
}

// 0xAC
void loadYAbsolute(CPU *cpu) {
  uint8_t value = fetchAbsolute(cpu);
  loadY(cpu, value);
}

// 0xBC
void loadYAbsoluteY(CPU *cpu) {
  uint8_t value = fetchAbsoluteY(cpu);
  loadY(cpu, value);
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
    // ASL
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
    // BPL oper, ex BPL 20
    branchOnPlusRelative(cpu);
    break;
  case (0x11):
    // ORA (oper),Y, ex ORA (20),Y
    orAIndirectY(cpu);
    break;
  case (0x15):
    // ORA oper,X
    orAZeroPageX(cpu);
    break;
  case (0x16):
    // ASL oper,X
    arithmeticShiftLeftZeroPageX(cpu);
    break;
  case (0x18):
    // CLC
    clearCarry(cpu);
  case (0x19):
    // ORA oper,Y
    orAAbsoluteY(cpu);
    break;
  case (0x1D):
    // ORA oper,X
    orAAbsoluteX(cpu);
    break;
  case (0x1E):
    // ASL oper,X
    arithmeticShiftLeftAbsoluteX(cpu);
    break;
  case (0x20):
    jumpSubRoutineAbsolute(cpu);
    break;
  case (0x21):
    andIndirectX(cpu);
    break;
  case (0x24):
    bitTestZeroPage(cpu);
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
    bitTestAbsolute(cpu);
    break;
  case (0x2D):
    // andAbsolute(cpu);
    break;
  case (0x2E):
    // rotateLeftAbsolute(cpu);
    break;
  case (0x30):
    branchOnMinusRelative(cpu);
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
    branchOnOverflowClearRelative(cpu);
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
    addWithCarryIndirectX(cpu);
    break;
  case (0x65):
    addWithCarryZeroPage(cpu);
    break;
  case (0x66):
    // rotateRightZeroPage(cpu);
    break;
  case (0x68):
    // pullAccumulatorFromStack(cpu);
    break;
  case (0x69):
    addWithCarryImmediate(cpu);
    break;
  case (0x6A):
    // rotateRightAccumulator(cpu);
    break;
  case (0x6C):
    // jumpIndirect(cpu);
    break;
  case (0x6D):
    addWithCarryAbsolute(cpu);
    break;
  case (0x6E):
    // rotateRightAbsolute(cpu);
    break;
  case (0x70):
    branchonOverflowSetRelative(cpu);
    break;
  case (0x71):
    addWithCarryIndirectY(cpu);
    break;
  case (0x75):
    addWithCarryZeroPageX(cpu);
    break;
  case (0x76):
    // rotateRightZeroPageX(cpu);
    break;
  case (0x78):
    // setInterruptDisable(cpu);
    break;
  case (0x79):
    addWithCarryAbsoluteY(cpu);
    break;
  case (0x7D):
    addWithCarryAbsoluteX(cpu);
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
    branchOnNotEqualRelative(cpu);
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
    branchOnEqualRelative(cpu);
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
