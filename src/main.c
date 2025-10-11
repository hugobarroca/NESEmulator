#define FONT_PATH "Sans.ttf"
#define SDL_MAIN_HANDLED

#include "SDL_keycode.h"
#include "emulator.h"
#include "libs/strings.h"
#include "utilities.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const SDL_Color White = {255, 255, 255};
const SDL_Color Black = {0, 0, 0};

int commandInteger;
char userSelection;
CPU cpu;
DIR *userDir;

SDL_Window *window;
SDL_Renderer *renderer;

void welcomeScreen() {
  struct dirent *dir;
  userDir = opendir(".");

  for (;;) {
    printf("Welcome to NESEmulator! Please select an option:\n");
    printf("0. Quit.\n");
    printf("1. Load game.\n");
    userSelection = getchar();
    if (userSelection == '0') {
      printf("Exiting program!\n");
      return;
    }
    if (userSelection == '1') {
      printf("Please type the name of the game you wish to load.\n");
      while ((dir = readdir(userDir)) != NULL) {
        if (dir->d_name[0] != '.') {
          printf("%s\n", dir->d_name);
        }
      }
      char gameName[250];
      resetInputBuffer();
      fgets(gameName, sizeof(gameName), stdin);
      char *p = strchr(gameName, '\n');
      if (p != NULL) {
        *p = '\0';
      }
      printf("The game you selected was: %s\n", gameName);
      loadGame(&cpu, gameName);
      printf("Emulator functionality to be developed.\n");
      return;
    }
    printf("Command not recognized.\n\n");
  }

  return;
}

int checkSdlInitErrors() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init failed: %s/n", SDL_GetError());
    return 1;
  }

  if (TTF_Init() != 0) {
    printf("TTF_Init failed: %s/n", TTF_GetError());
    return 1;
  }

  return 0;
}

void drawCurrInstLabel() {
  TTF_Font *font = TTF_OpenFont(FONT_PATH, 24);
  char currInstLabel[256] = "Current instruction: \n";
  char secBuf[256];
  snprintf(secBuf, sizeof(secBuf), "%s",
           getInstructionName(&cpu, getCurrentInstruction(&cpu)));
  strncat(currInstLabel, secBuf,
          sizeof(currInstLabel) - strlen(currInstLabel) - 1);
  printf("Current ins label: %s", currInstLabel);
  fflush(stdout);
  SDL_Surface *currIntSurfaceMessage =
      TTF_RenderUTF8_Solid(font, currInstLabel, White);
  if (currIntSurfaceMessage == NULL) {
    printf("TTF_RenderUTF8_Solid failed: %s\n", TTF_GetError());
  }
  SDL_Texture *currInstLabelTexture =
      SDL_CreateTextureFromSurface(renderer, currIntSurfaceMessage);

  int xPosition, yPosition, width, height;
  int rendererWidth, rendererHeight;
  SDL_GetRendererOutputSize(renderer, &rendererWidth, &rendererHeight);
  printf("Renderer output size: %dx%d\n", rendererWidth, rendererHeight);
  fflush(stdout);

  xPosition = 20;
  yPosition = rendererHeight - (rendererHeight / 10) + 10;
  width = rendererWidth / 6;
  height = rendererHeight / 10 - 35;
  SDL_Rect pcRect = {xPosition, yPosition, width, height};
  SDL_Rect currInstRect = {xPosition + width + 5, yPosition, width, height};

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  int renderSuccess2 =
      SDL_RenderCopy(renderer, currInstLabelTexture, NULL, &currInstRect);

  SDL_RenderPresent(renderer);
}

void runMainSDLLoop() {
  SDL_Event event;
  int running = 1;
  while (running) {
    fflush(stdout);
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        printf("QUIT event was issued!\n");
        running = 0;
      } else if (event.type == SDL_KEYDOWN) {
        printf("Key was pressed! Code: %u\n", event.key.keysym.sym);
        if (event.key.keysym.sym == SDLK_KP_ENTER) {
          printf("> ENTER was pressed. Executing instruction.");
          executeInstruction(&cpu);
        } else if (event.key.keysym.sym == SDLK_RETURN) {
          printf("ENTER was pressed.\n");
          executeInstruction(&cpu);
        }
      }
    }

    //drawCurrInstLabel();
		// printf("Current pc %u\n", cpu.PC);
    SDL_Delay(16);
  }
}

void createUI() {
  int sdlStartedSuccessfully = checkSdlInitErrors();
  if (sdlStartedSuccessfully != 0) {
    printf("SDL did not start sucessfully, check above errors.");
    return;
  }

  window = SDL_CreateWindow("Scald Emulator", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN);
  if (!window) {
    printf("SDL_CreateWindow failed: %s`n", SDL_GetError());
    SDL_Quit();
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return;
  }

  TTF_Font *font = TTF_OpenFont(FONT_PATH, 24);

  char programCounterLabelText[100];
  appendIntToString("Program Counter: ", cpu.PC, programCounterLabelText,
                    sizeof(programCounterLabelText));
  SDL_Surface *programCounterLabelSurface =
      TTF_RenderUTF8_Solid(font, programCounterLabelText, White);
  if (programCounterLabelSurface == NULL) {
    printf("TTF_RenderUTF8_Solid failed: %s\n", TTF_GetError());
    printf("Check if Sans.ttf exists in build directory.");
  }

  // char stackPointerLabelText[256] = "Stack pointer: ";
  // char buffer[256];
  // snprintf(buffer, sizeof(buffer), "%u", getStackPointerValue(&cpu));
  // fflush(stdout);
  // strncat(stackPointerLabelText, buffer,
  //         sizeof(stackPointerLabelText) - strlen(stackPointerLabelText) - 1);

  char currInstLabel[256] = "Current instruction: \n";
  char secBuf[256];
  snprintf(secBuf, sizeof(secBuf), "%s",
           getInstructionName(&cpu, getCurrentInstruction(&cpu)));
  strncat(currInstLabel, secBuf,
          sizeof(currInstLabel) - strlen(currInstLabel) - 1);
  printf("Current ins label: %s", currInstLabel);
  fflush(stdout);
  SDL_Surface *currIntSurfaceMessage =
      TTF_RenderUTF8_Solid(font, currInstLabel, White);
  if (currIntSurfaceMessage == NULL) {
    printf("TTF_RenderUTF8_Solid failed: %s\n", TTF_GetError());
  }

  SDL_Texture *pcLabelTexture =
      SDL_CreateTextureFromSurface(renderer, programCounterLabelSurface);
  if (pcLabelTexture == NULL) {
    printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
  }

  SDL_Texture *currInstLabelTexture =
      SDL_CreateTextureFromSurface(renderer, currIntSurfaceMessage);
  if (pcLabelTexture == NULL) {
    printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
  }

  SDL_SetRenderDrawColor(renderer, 24, 26, 24, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 100, 103, 100, 255);
  int rendererWidth, rendererHeight;
  SDL_GetRendererOutputSize(renderer, &rendererWidth, &rendererHeight);
  printf("Renderer output size: %dx%d\n", rendererWidth, rendererHeight);
  fflush(stdout);

  SDL_Rect footerBorder = {0, rendererHeight - (rendererHeight / 10),
                           rendererWidth, rendererHeight / 10};
  SDL_Rect footerInnerRect = {5, rendererHeight - (rendererHeight / 10) + 5,
                              rendererWidth - 10, rendererHeight / 10 - 10};

  int xPosition, yPosition, width, height;
  xPosition = 20;
  yPosition = rendererHeight - (rendererHeight / 10) + 10;
  width = rendererWidth / 6;
  height = rendererHeight / 10 - 35;

  SDL_Rect pcRect = {xPosition, yPosition, width, height};
  SDL_Rect currInstRect = {xPosition + width + 5, yPosition, width, height};

  SDL_RenderFillRect(renderer, &footerBorder);
  SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);

  SDL_RenderFillRect(renderer, &footerInnerRect);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  int renderSuccess = SDL_RenderCopy(renderer, pcLabelTexture, NULL, &pcRect);
  int renderSuccess2 =
      SDL_RenderCopy(renderer, currInstLabelTexture, NULL, &currInstRect);

  SDL_RenderPresent(renderer);
  loadGame(&cpu, "dk.nes");

  runMainSDLLoop();

  SDL_DestroyWindow(window);
  SDL_Quit();
  return;
}

struct ThreadArgs {
  struct CPU *cpu;
};

void *loadAndTestGame(void *arg) {
  // struct ThreadArgs *args = (struct ThreadArgs *)arg;
  char gameName[] = "dk";
  // loadGame(args->cpu, gameName);
  loadGame(&cpu, gameName);
  return NULL;
}

void createCPUThread() {
  // pthread_t cpuThread;
  // struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
  // args->cpu = &cpu;
  // initializeInstructionArray(&cpu);
  // pthread_create(&cpuThread, NULL, loadAndTestGame, &args);
	// Running synchronously for now
	initializeInstructionArray(&cpu);				
	loadAndTestGame(NULL);
}

void runUIAndCPUThreads() {
  createCPUThread();
  createUI();
}

int main(int argc, char *argv[]) { runUIAndCPUThreads(); }
