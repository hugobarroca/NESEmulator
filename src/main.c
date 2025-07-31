#define SDL_MAIN_HANDLED
#include "emulator.h"
#include "utilities.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int commandInteger;
char userSelection;
CPU cpu;
DIR *userDir;

void *welcomeScreen() {
  struct dirent *dir;
  userDir = opendir(".");

  for (;;) {
    printf("Welcome to NESEmulator! Please select an option:\n");
    printf("0. Quit.\n");
    printf("1. Load game.\n");
    userSelection = getchar();
    if (userSelection == '0') {
      printf("Exiting program!\n");
      return 0;
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
      return 0;
    }
    printf("Command not recognized.\n\n");
  }

  return 0;
}

void appendIntToString(char *prefix, int value, char *target) {
  snprintf(target, sizeof(target), "%s%d", prefix, value);
}

int checkInitErrors() {
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

void *createWindow(void *arg) {
  int isInitSuccess = checkInitErrors();

  if (isInitSuccess != 0) {
    return NULL;
  }

  SDL_Window *window =
      SDL_CreateWindow("My SDL2 Window", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return NULL;
  }
  if (!window) {
    printf("SDL_CreateWindow failed: %s`n", SDL_GetError());
    SDL_Quit();
    return NULL;
  }

  int running = 1;
  SDL_Event event;

  // This, unsurprisingly, requires the path to actually point to a ttf file...
  TTF_Font *Sans = TTF_OpenFont("Sans.ttf", 24);
  SDL_Color White = {255, 255, 255};

  char result[100];
  // appendIntToString("Stack pointer: ", cpu.PC, result);
  //  SDL_Surface *surfaceMessage =
  TTF_RenderUTF8_Solid(Sans, result, White);

  char message[256] = "Stack pointer: ";
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%u", getStackPointerValue(&cpu));
  strncat(message, buffer, sizeof(message) - strlen(message) - 1);

  char currInstMessage[256] = "Current instruction: ";
  char secBuf[256];
  snprintf(secBuf, sizeof(secBuf), "%s",
           getInstructionName(getCurrentInstruction(&cpu)));
  strncat(currInstMessage, secBuf,
          sizeof(currInstMessage) - strlen(currInstMessage) - 1);

  SDL_Surface *surfaceMessage = TTF_RenderUTF8_Solid(Sans, message, White);
  if (surfaceMessage == NULL) {
    printf("TTF_RenderUTF8_Solid failed: %s\n", TTF_GetError());
  }

  SDL_Surface *currIntSurfaceMessage =
      TTF_RenderUTF8_Solid(Sans, currInstMessage, White);
  if (currIntSurfaceMessage == NULL) {
    printf("TTF_RenderUTF8_Solid failed: %s\n", TTF_GetError());
  }

  SDL_Texture *topStackValueTexture =
      SDL_CreateTextureFromSurface(renderer, surfaceMessage);

  SDL_Texture *currInstTexture =
      SDL_CreateTextureFromSurface(renderer, currIntSurfaceMessage);

  if (topStackValueTexture == NULL) {
    printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
  }

  SDL_SetRenderDrawColor(renderer, 24, 26, 24, 255);
  SDL_RenderClear(renderer);

  SDL_Rect rect = {0, 400, 800, 130};
  SDL_SetRenderDrawColor(renderer, 100, 103, 100, 255);

  int rw, rh;
  SDL_GetRendererOutputSize(renderer, &rw, &rh);
  printf("Renderer output size: %dx%d\n", rw, rh);

  SDL_Rect footerBorder = {0, rh - (rh / 10), rw, rh / 10};
  SDL_Rect footerInnerRect = {5, rh - (rh / 10) + 5, rw - 10, rh / 10 - 10};

  int xPosition, yPosition, width, height;
  xPosition = 20;
  yPosition = rh - (rh / 10) + 10;
  width = rw / 6 - 10;
  height = rh / 10 - 35;

  SDL_Rect stackLabel = {xPosition, yPosition, width, height};
  SDL_Rect currentInstructionLabel = {xPosition + width + 5, yPosition, width,
                                      height};

  SDL_RenderFillRect(renderer, &footerBorder);
  SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);

  SDL_RenderFillRect(renderer, &footerInnerRect);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  int renderSuccess =
      SDL_RenderCopy(renderer, topStackValueTexture, NULL, &stackLabel);

  int renderSuccess2 =
      SDL_RenderCopy(renderer, currInstTexture, NULL, &currentInstructionLabel);
  if (renderSuccess != 0) {
    printf("SDL_RenderCopy failed: %s`n", TTF_GetError());
    SDL_Quit();
    return NULL;
  }

  SDL_RenderPresent(renderer);

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }

    SDL_Delay(16);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

struct ThreadArgs {
  struct CPU *cpu;
};

void *loadAndTestGame(void *arg) {
  struct ThreadArgs *args = (struct ThreadArgs *)arg;
  char gameName[] = "dk";
  loadGame(args->cpu, gameName);
  return NULL;
}

void *initHardwareAndUi() {
  // Using one thread for the UI, one for the emulator (for now)
  pthread_t thread1;
  struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
  args->cpu = &cpu;
  initializeInstructionArray();
	
  pthread_create(&thread1, NULL, loadAndTestGame, &args);
  createWindow(NULL);
  return 0;
}

int main(int argc, char *argv[]) { initHardwareAndUi(); }
