#include "cartridge.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int loadGame(Cartridge *cartridge, char *fileName) {
  FILE *file = fopen(fileName, "rb");
  if (file == NULL) {
    printf("ERROR: File not found.\n");
    return 0;
  }
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  printf("Opened file successfully, filesize: %ld bytes\n", fileSize);
  cartridge->GameData = malloc(fileSize);
  fread(cartridge->GameData, sizeof(uint8_t), (fileSize), file);
  return 1;
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
    return 0;
  }

  printf("Unknown mapper type detected: %d\n", mapperId);
  return 1;
}
