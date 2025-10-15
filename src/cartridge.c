#include "cartridge.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int loadGame(Cartridge *cartridge, char *fileName) {
  FILE *file = fopen(fileName, "rb");
  if (file == NULL) {
    printf("ERROR: File not found.\n");
    return 1;
  }
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  printf("Opened file successfully, filesize: %ld bytes\n", fileSize);
  cartridge->GameData = malloc(fileSize);
  fread(cartridge->GameData, sizeof(uint8_t), (fileSize), file);
  return 0;
}

int detectRomFormat(Cartridge *cartridge) {
  // For the iNES format, the header takes 20 bytes
  // The 7th and 8th bytes tell us the mapper type for the cartridge.
  // 7th byte - 4 leftmost digits are the lower nybble of the mapper
  // 8th byte - 4 leftmost digits are the upper nybble of the mapper
  uint8_t byteSeven = cartridge->GameData[8];
  uint8_t byteEight = cartridge->GameData[8];

  // 1100 1100 (keep only lower half, so and with 11110000)
  // 1010 1010 (keep only lower half, so and with 11110000)
  //
  uint8_t lowerNybble = byteSeven && 0xF0;
  uint8_t upperNybble = byteEight && 0xF0;
  uint8_t mapperId = upperNybble && lowerNybble;

  if ((byteSeven & 0x0C) == 0x04) {
    printf("Archaic iNES format detected.\n");
    return 0;
  }

  if (mapperId == 0) {
    printf("Archaic iNES format detected. (Id=0)\n");
    cartridge->MapperType = 0;
    return 0;
  }

  printf("Unknown mapper type detected: %d\n", mapperId);
  return 1;
}

void printHeaderInformation(Cartridge *cartridge) {
  printf("HEADER START: %.3s\n", cartridge->GameData);
  printf("PRG ROM Size: %d KBs\n", cartridge->GameData[4] * 16);
  printf("CHR ROM Size: %d KBs\n", cartridge->GameData[5] * 8);

  // Flags 6
  // printf("Flags 6: %d\n", cartridge->GameData[6]);
  printf("=== Flags 6 ===\n");
  if ((cartridge->GameData[6] & 0x01) == 0x01) {
    printf("Nametable arrangement: horizontal\n");
  } else {
    printf("Nametable arrangement: vertical\n");
  }
  if ((cartridge->GameData[6] & 0x02) == 0x02) {
    printf("Battery-backed PRG RAM detected.\n"); // usually $6000
  } else {
    printf("No persistent memory detected.\n");
  }
  if ((cartridge->GameData[6] & 0x04) == 0x04) {
    printf("512-byte trainer is present!\n");
    cartridge->IsTrainerPresent = true;
  } else {
    printf("No trainer present.\n");
    cartridge->IsTrainerPresent = false;
  }
  if ((cartridge->GameData[6] & 0x08) == 0x08) {
    printf("Using alternative nametable layout!\n");
  } else {
    printf("Regular nametable layout.\n");
  }

  // Flags 7
  // printf("Flags 7: %d\n", cartridge->GameData[7]);
  printf("=== Flags 7 ===\n");
  if ((cartridge->GameData[7] & 0x01) == 0x01) {
    printf("VS Unisystem ON\n");
  } else {
    printf("VS Unisystem OFF\n");
  }
  if ((cartridge->GameData[7] & 0x02) == 0x02) {
    printf("PlayChoice-10 is on.\n");
  } else {
    printf("PlayChoice-10 is off.\n");
  }

  uint8_t mapperNumber =
      (cartridge->GameData[7] & 0xF0) + ((cartridge->GameData[6] & 0xF0) >> 4);
  // cpu->MapperType = mapperNumber;
  // setAndPrintMapper(cpu, mapperNumber);
  printf("Mapper number: %d\n", mapperNumber);
  // Flags 8
  printf("PRG RAM size: 0x%02x\n", cartridge->GameData[8]);
  // Flags 9
  printf("Flags 9: %d\n", cartridge->GameData[9]);
  if ((cartridge->GameData[9] & 0x01) == 1) {
    printf("TV System: PAL\n");
  } else {
    printf("TV System NTSC\n");
  }
  // Flags 10
  printf("Flags 10: %d\n", cartridge->GameData[10]);
  if ((cartridge->GameData[10] & 0x02) == 0x00) {
    printf("TV System: NTSC\n");
  } else if ((cartridge->GameData[10] & 0x02) == 0x01) {
    printf("TV System: PAL\n");
  } else {
    printf("TV System: Dual Compatible!\n");
  }
  uint8_t *ripper = cartridge->GameData + 11;
  printf("RIPPER NAME: %.5s\n", ripper);
}

int loadPrgRom(Cartridge *cartridge) {
  if (cartridge->MapperType == NROM_128) {
    int RomSize = 16 * 1024;
    cartridge->PRG_ROM = malloc(RomSize);
    uint8_t *ptrPosition = cartridge->GameData;
    ptrPosition += 16; // Skip first 16 bytes of header
    if (cartridge->IsTrainerPresent) {
      ptrPosition += 512; // Skip first 512 bytes of trainer
    }
    for (int i = 0; i < RomSize; i++) {
      cartridge->PRG_ROM[i] = *ptrPosition;
      ptrPosition++;
    }
  }
  return 0;
}

// The address is the CPU mapped address
uint8_t getValueAtAddress(Cartridge *cartridge, uint16_t address) {
  if (cartridge->MapperType == NROM_128) {
    if (address >= 0x8000 && address <= 0xBFFF) {
      uint16_t cartridgeAddress = address - 0x8000 / 16;
      uint8_t *value = cartridge->PRG_ROM + cartridgeAddress;
      return *value;
    }
  }
}
