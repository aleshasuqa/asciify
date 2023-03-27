#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>

// compute the average brightness of a cell
int __get_average_brightness(SDL_Surface *cell) {
    int w = cell->w;
    int h = cell->h;
    Uint32 *pixels = (Uint32*)cell->pixels;
    int sum = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Uint32 pixel = pixels[i*w+j];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, cell->format, &r, &g, &b);
            sum += (r + g + b) / 3;
        }
    }
    return sum / (w*h);
}

// compute variance of a cell
int get_variance(SDL_Surface *cell) {
    int w = cell->w;
    int h = cell->h;
    Uint32 *pixels = (Uint32*)cell->pixels;
    int sum = 0;
    int average = __get_average_brightness(cell);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Uint32 pixel = pixels[i*w+j];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, cell->format, &r, &g, &b);
            sum += pow((r + g + b) / 3 - average, 2);
        }
    }
    return sum / (w*h);
}

// compute covariance of two cells
int get_covariance(SDL_Surface *cell1, SDL_Surface *cell2) {
    int w = cell1->w;
    int h = cell1->h;
    Uint32 *pixels1 = (Uint32*)cell1->pixels;
    Uint32 *pixels2 = (Uint32*)cell2->pixels;
    int sum = 0;
    int average1 = __get_average_brightness(cell1);
    int average2 = __get_average_brightness(cell2);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Uint32 pixel1 = pixels1[i*w+j];
            Uint32 pixel2 = pixels2[i*w+j];
            Uint8 r1, g1, b1;
            Uint8 r2, g2, b2;
            SDL_GetRGB(pixel1, cell1->format, &r1, &g1, &b1);
            SDL_GetRGB(pixel2, cell2->format, &r2, &g2, &b2);
            sum += ((r1 + g1 + b1) / 3 - average1) * ((r2 + g2 + b2) / 3 - average2);
        }
    }
    return sum / (w*h);
}

// compute the SSIM of two cells
double get_ssim(SDL_Surface *cell1, SDL_Surface *cell2) {
    double k1 = 0.01;
    double k2 = 0.03;
    double L = 255;
    double c1 = pow((k1*L), 2);
    double c2 = pow((k2*L), 2);
    int average1 = __get_average_brightness(cell1);
    int average2 = __get_average_brightness(cell2);
    int variance1 = get_variance(cell1);
    int variance2 = get_variance(cell2);
    int covariance = get_covariance(cell1, cell2);
    double ssim = ((2*average1*average2 + c1) * (2*covariance + c2)) /
            ((pow(average1, 2) + pow(average2, 2) + c1) * (variance1 + variance2 + c2));
    return ssim;
}