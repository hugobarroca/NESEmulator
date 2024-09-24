// This file is meant to hold utility functions
#include "utilities.h"
#include <dirent.h>
#include <stdio.h>
#include <regex.h>

void resetInputBuffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

int countMatchingFiles(char *directory, char *match) {
  DIR *d = opendir(directory);
  struct dirent *dir;

  int count = 0;
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_name[0] != '.') {
      count++;
    }
  }
  closedir(d);
  return count;
}
