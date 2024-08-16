#include <string.h>
#include <stdio.h>
#include "cpu.h"
#include "utilities.h"

int welcomeScreen(){	
	int commandInteger;
	char c;
	CPU cpu;

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
			printf("Please type the name of the game you wish to load.\n");
			char gameName[250];
			resetInputBuffer();
			fgets(gameName, sizeof(gameName), stdin);
			char *p = strchr(gameName, '\n');
			if(p != NULL){
				*p = '\0';	
			}
			printf("The game you selected was: %s\n", gameName);
			loadGame(&cpu, gameName);
			printf("Emulator functionality to be developed.\n");
			return 0;
		}
		printf("Command not recognized.\n\n");
	}

	return 0;
}

int main(int argc, char *argv[]){
	welcomeScreen();
	return 0;
}

