#include "../cartridge.c"

void testLoadGame(Cartridge *c) {
  char *gameName = "dk.nes";
  loadGame(c, gameName);
}

void testDetectRomFormat(Cartridge *c) { detectRomFormat(c); }

int main() {
  Cartridge c = {};
  testLoadGame(&c);
  testDetectRomFormat(&c);
  return 0;
}
