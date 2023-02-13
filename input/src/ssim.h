#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double ssim(SDL_Surface* image1, SDL_Surface* image2);
SDL_Surface* convert_to_grayscale(SDL_Surface* image);
SDL_Surface* read_image(char* filename);