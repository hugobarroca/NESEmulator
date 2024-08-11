#include <stdio.h>

struct Stack{
	int stack[64];
	int* stackPointer;
};

void initStack(struct Stack *s){
	s->stackPointer = s->stack - 1;
}

void pushStack(struct Stack *s, int value){
	s->stackPointer++;
	*(s->stackPointer) = value;
}

int popStack(struct Stack *s){
	if(s->stackPointer < s->stack)
	{
		printf("ERROR: Stack underflow detected!\n");
		return -1;
	}
	int value = *(s->stackPointer);
	s->stackPointer--;
	return value;
}

