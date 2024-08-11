// This file is meant to hold all the necessary code to emulate the Ricoh 2A03 CPU (based on the 6502 CPU).
#include <stdbool.h>
#include "stack.h"

void runProcessor(){
	int programCounter[2];
	Stack s;
	initStack(&s);

	while(true){
		//fetch instruction from memory
		//execute instruction
	}
}
