#ifndef STACK_H
#define STACK_H

typedef struct {
	int stack[64];
	int* stackPointer;
} Stack;

void initStack(Stack *s);
void pushStack(Stack *s, int value);
int popStack(Stack *s);

#endif
