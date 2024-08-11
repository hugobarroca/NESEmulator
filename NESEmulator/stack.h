#ifndef STACK_H
#define STACK_H

struct Stack{
	int stack[64];
	int* stackPointer;
};

void initStack(struct Stack *s);
void pushStack(struct Stack *s, int value);
int popStack(struct Stack *s);

#endif
