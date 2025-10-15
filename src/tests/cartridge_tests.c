#include "../cartridge.c"

void testLoadGame(Cartridge *c) {
  char *gameName = "dk.nes";
  loadGame(c, gameName);
}

void testDetectRomFormat(Cartridge *c) { detectRomFormat(c); }

void testPrintHeaderInformation(Cartridge *c) { printHeaderInformation(c); }

int main() {
  Cartridge c = {};
  testLoadGame(&c);
  testDetectRomFormat(&c);
  testPrintHeaderInformation(&c);
  return 0;
}
