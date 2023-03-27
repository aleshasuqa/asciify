#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int get_average_brightness(SDL_Surface *cell);
void draw(SDL_Renderer* renderer, SDL_Texture* texture);
void event_loop(SDL_Renderer* renderer, SDL_Texture* colored);
SDL_Surface* __scale(SDL_Surface* image, int x, int y);


