#include <stdint.h>

typedef struct {
  uint8_t *GameData;
} Cartridge;

int loadGame(Cartridge *cartridge, char *filename);

int detectRomFormat(Cartridge *cartridge);
