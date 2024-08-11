// This file is meant to hold utility functions
#include "utilities.h"
#include <stdio.h>

void resetInputBuffer(){
	int c;
	while(( c = getchar()) != '\n' && c != EOF);
}

