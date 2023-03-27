#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double get_ssim(SDL_Surface *cell1, SDL_Surface *cell2);