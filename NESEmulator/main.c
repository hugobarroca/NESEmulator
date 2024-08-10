#include <stdio.h>

void resetInputBuffer(){
	int c;
	while(( c = getchar()) != '\n' && c != EOF);
}

void loadGame(){
	FILE *file = fopen("", "r");
	fclose(file);
}

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
			printf("Please type the name of the game you wish to load.\n");
			char gameName[50];
			resetInputBuffer();
			fgets(gameName, sizeof(gameName), stdin);
			printf("The game you selected was: %s", gameName);
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

