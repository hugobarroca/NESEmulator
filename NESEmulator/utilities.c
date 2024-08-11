// This file is meant to hold utility functions
#include <stdio.h>
#include "utilities.h"

void resetInputBuffer(){
	int c;
	while(( c = getchar()) != '\n' && c != EOF);
}

