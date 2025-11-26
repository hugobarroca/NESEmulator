#include "../cartridge.c"

void testLoadGame(Cartridge *c) {
  char *gameName = "dk.nes";
  loadGame(c, gameName);
}

void testDetectRomFormat(Cartridge *c) {
  printf("====== RUNNING TEST 1 ======\n");
  detectRomFormat(c);
  printf("====== FINISHED TEST 1 ======\n");
  printf("\n");
}

void testPrintHeaderInformation(Cartridge *c) {
  printf("====== RUNNING TEST 2 ======\n");
  printHeaderInformation(c);
  printf("====== FINISHED TEST 2 ======\n");
  printf("\n");
}

int main() {
  Cartridge c = {};
  testLoadGame(&c);
  testDetectRomFormat(&c);
  testPrintHeaderInformation(&c);
  return 0;
}
