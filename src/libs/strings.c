#include <stdio.h>

void appendIntToString(char *prefix, int value, char *resultBuffer,
                       int resultBufferSize) {
  snprintf(resultBuffer, resultBufferSize, "%s%d", prefix, value);
  printf("Program counter label: \"%s\"\n", resultBuffer);
  fflush(stdout);
}
