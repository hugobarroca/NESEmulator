#define SDL_MAIN_HANDLED
#include "emulator.h"
#include "utilities.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

int welcomeScreen() {
  int commandInteger;
  char c;
  CPU cpu;
  DIR *d;
  struct dirent *dir;
  d = opendir(".");

  for (;;) {
    printf("Welcome to NESEmulator! Please select an option:\n");
    printf("0. Quit.\n");
    printf("1. Load game.\n");
    c = getchar();
    if (c == '0') {
      printf("Exiting program!\n");
      return 0;
    }
    if (c == '1') {
      printf("Please type the name of the game you wish to load.\n");
      while ((dir = readdir(d)) != NULL) {
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

int createWindow() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init failed: %s/n", SDL_GetError());
  }
  TTF_Init();
  SDL_Window *window =
      SDL_CreateWindow("My SDL2 Window", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  if (!window) {
    printf("SDL_CreateWindow failed: %s`n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  int running = 1;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }

    TTF_Font* Sans = TTF_OpenFont("Sans.ttf", 24);
		SDL_Color White = {255, 255, 255};
		SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "Somethingaaa", White);
		SDL_Texture* textureMessage = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_SetRenderDrawColor(renderer, 24, 26, 24, 255);
    SDL_RenderClear(renderer);

    SDL_Rect rect = {0, 500, 800, 100};
    SDL_SetRenderDrawColor(renderer, 100, 103, 100, 255);
    SDL_RenderFillRect(renderer, &rect);
		SDL_RenderCopy(renderer, textureMessage, NULL, &rect);

    SDL_RenderPresent(renderer);

    SDL_Delay(16);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

int main(int argc, char *argv[]) {
  createWindow();
  welcomeScreen();
  return 0;
}
