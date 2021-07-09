#include <stdio.h>

int main(int argc, char *argv[]){
    int commandInteger;
    for(;;){
        printf("Welcome to NESEmulator! Please select an option:\n");
        printf("1. Quit.\n");
        scanf("%d", &commandInteger);
        if(commandInteger == 1){
            printf("Exiting program!\n");
            return 0;
        }else{
            printf("Command not recognized.");
        }
    }

    return 0;
}
