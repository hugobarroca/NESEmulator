#include "emulator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void detectGameFormat(CPU *cpu) {
  uint8_t byteSeven = cpu->Memory[7];
  if ((byteSeven & 0x0C) == 0x04) {
    printf("Archaic iNES format detected.\n");
    return;
  }
  bool endBytesEmpty = true;
  int i = 12;
  while (i < 16) {
    if (cpu->Memory[i] != 0) {
      endBytesEmpty = false;
    }
    i++;
  }
  if (((byteSeven & 0x0C) == 0x00) && endBytesEmpty) {
    printf("Archaic iNES format detected.\n");
    return;
  }
  printf("Unknown game format detected.\n");
}

void readGameHeader(CPU *cpu) {
  uint8_t *nes = cpu->Memory;
  printf("HEADER START: %.3s\n", nes);
  printf("PRG ROM Size: %d KBs.\n", cpu->Memory[4] * 16);
  printf("CHR ROM Size: %d KBs.\n", cpu->Memory[5] * 8);

  // Flags 6
  printf("Flags 6: %d\n", cpu->Memory[6]);
  if ((cpu->Memory[6] & 0x01) == 0x01) {
    printf("Nametable arrangement: horizontal\n");
  } else {
    printf("Nametable arrangement: vertical\n");
  }
  if ((cpu->Memory[6] & 0x02) == 0x02) {
    printf("Battery-backed PRG RAM detected.\n"); // usually $6000
  } else {
    printf("No persistent memory detected.\n");
  }
  if ((cpu->Memory[6] & 0x04) == 0x04) {
    printf("512-byte trainer is present!\n");
  } else {
    printf("No trainer present.\n");
  }
  if ((cpu->Memory[6] & 0x08) == 0x08) {
    printf("Using alternative nametable layout!\n");
  } else {
    printf("Regular nametable layout.\n");
  }

  // Flags 7
  printf("Flags 7: %d\n", cpu->Memory[7]);
  if ((cpu->Memory[7] & 0x01) == 0x01) {
    printf("VS Unisystem ON\n");
  } else {
    printf("VS Unisystem OFF\n");
  }
  if ((cpu->Memory[7] & 0x02) == 0x02) {
    printf("PlayChoice-10 is on.\n");
  } else {
    printf("PlayChoice-10 is off.\n");
  }

  uint8_t mapperNumber =
      (cpu->Memory[7] & 0xF0) + ((cpu->Memory[6] & 0xF0) >> 4);
  cpu->MapperType = mapperNumber;
  setAndPrintMapper(cpu, mapperNumber);
  printf("Mapper number: %d\n", mapperNumber);
  // Flags 8
  printf("PRG RAM size: 0x%02x\n", cpu->Memory[8]);
  // Flags 9
  printf("Flags 9: %d\n", cpu->Memory[9]);
  if ((cpu->Memory[9] & 0x01) == 1) {
    printf("TV System: PAL\n");
  } else {
    printf("TV System NTSC\n");
  }
  // Flags 10
  printf("Flags 10: %d\n", cpu->Memory[10]);
  if ((cpu->Memory[10] & 0x02) == 0x00) {
    printf("TV System: NTSC\n");
  } else if ((cpu->Memory[10] & 0x02) == 0x01) {
    printf("TV System: PAL\n");
  } else {
    printf("TV System: Dual Compatible!\n");
  }
  uint8_t *ripper = cpu->Memory + 11;
  printf("RIPPER NAME: %.5s\n", ripper);
}

void loadGame(CPU *cpu, char fileName[]) {
  printf("Attempting to load game: %s\n", fileName);
  FILE *file = fopen(fileName, "rb");
  if (file == NULL) {
    printf("File not found.\n");
    return;
  }
  printf("Opened the file successfully.\n");
  //
  // Set cursor to end of file to read length
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);

  // Reset cursor to beggining
  fseek(file, 0, SEEK_SET);

  printf("Filesize: %ld bytes\n", fileSize);
  // Set gameData array size
  cpu->GameData = malloc(fileSize / 8);
  fread(cpu->GameData, sizeof(uint8_t), (fileSize / 8), file);
  printf("Read file successfully!\n");

  initProcessor(cpu);
  readGameHeader(cpu);
  detectGameFormat(cpu);
  execute(cpu);
  fclose(file);
}
