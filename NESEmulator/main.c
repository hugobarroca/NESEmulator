#include <stdio.h>

int welcomeScreen(){	
	int commandInteger;
	char c;

	for(;;){
		printf("Welcome to NESEmulator! Please select an option:\n");
		printf("0. Quit.\n");
		printf("1. Load game.\n");
		c = getchar();
		if(c == '0'){
			printf("Exiting program!\n");
			return 0;
		}
		if(c == '1'){
			printf("This functionality has not yet been implemented.\n");
			continue;
		}
		printf("Command not recognized.\n\n");
	}

	return 0;
}

int main(int argc, char *argv[]){
	welcomeScreen();
	return 0;
}

