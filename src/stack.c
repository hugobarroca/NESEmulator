#include <stdio.h>
#include "stack.h"

void initStack(Stack *s){
	s->stackPointer = s->stack - 1;
}

void pushStack(Stack *s, int value){
	s->stackPointer++;
	*(s->stackPointer) = value;
}

int popStack(Stack *s){
	if(s->stackPointer < s->stack)
	{
		printf("ERROR: Stack underflow detected!\n");
		return -1;
	}
	int value = *(s->stackPointer);
	s->stackPointer--;
	return value;
}

