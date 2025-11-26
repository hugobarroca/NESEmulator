#include <stdbool.h>
#include <stdint.h>

enum Mapper { NROM_128 = 0 };

typedef struct {
  uint8_t *GameData;
  uint8_t *PRG_ROM;
  enum Mapper MapperType;
  bool IsTrainerPresent;

} Cartridge;

int loadGame(Cartridge *cartridge, char *filename);

int detectRomFormat(Cartridge *cartridge);

int loadPrgRom(Cartridge *cartridge);

void printHeaderInformation(Cartridge *cartridge);

uint8_t getValueAtAddress(Cartridge *cartridge, uint16_t address);
