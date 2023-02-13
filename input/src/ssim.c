#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>

// function that reads the image and returns it
SDL_Surface* read_image(char* filename) {
    SDL_Surface* image = IMG_Load(filename);
    if (image == NULL) {
        errx(EXIT_FAILURE, "IMG_Load: %s", IMG_GetError());
    }
    return image;
}

// get the pixel value at (x, y)
Uint32 get_pixel(SDL_Surface* surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *(Uint16*)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4:
            return *(Uint32*)p;
        default:
            return 0;
    }
}

// put the pixel value at (x, y)
void put_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    switch (bpp) {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(Uint16*)p = pixel;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4:
            *(Uint32*)p = pixel;
            break;
    }
}

// convert the image to grayscale
SDL_Surface* convert_to_grayscale(SDL_Surface* image) {
    SDL_Surface* gray = SDL_CreateRGBSurface(0, image->w, image->h, 32, 0, 0, 0, 0);
    if (gray == NULL) {
        errx(EXIT_FAILURE, "SDL_CreateRGBSurface: %s", SDL_GetError());
    }
    for (int i = 0; i < image->h; i++) {
        for (int j = 0; j < image->w; j++) {
            Uint32 pixel = get_pixel(image, j, i);
            Uint8 r, g, b;
            SDL_GetRGB(pixel, image->format, &r, &g, &b);
            Uint8 gray_value = 0.2126 * r + 0.7152 * g + 0.0722 * b;
            Uint32 gray_pixel = SDL_MapRGB(gray->format, gray_value, gray_value, gray_value);
            put_pixel(gray, j, i, gray_pixel);
        }
    }
    return gray;
}

// calculate weighted ssim for two images
double ssim(SDL_Surface* image1, SDL_Surface* image2) {
    double C1 = 0.01 * 255 * 0.01 * 255;
    double C2 = 0.03 * 255 * 0.03 * 255;
    double C3 = C2 / 2;
    double mean1 = 0;
    double mean2 = 0;
    double var1 = 0;
    double var2 = 0;
    double cov = 0;
    for (int i = 0; i < image1->h; i++) {
        for (int j = 0; j < image1->w; j++) {
            //printf("i = %d, j = %d \n", i, j);
            Uint32 pixel1 = get_pixel(image1, j, i);
            Uint32 pixel2 = get_pixel(image2, j, i);

            Uint8 r1, g1, b1;
            Uint8 r2, g2, b2;
            SDL_GetRGB(pixel1, image1->format, &r1, &g1, &b1);
            SDL_GetRGB(pixel2, image2->format, &r2, &g2, &b2);
            //double gray1 = 0.2126 * r1 + 0.7152 * g1 + 0.0722 * b1;
            //double gray2 = 0.2126 * r2 + 0.7152 * g2 + 0.0722 * b2;
            double gray1 = (r1 + g1 + b1) / 3;
            double gray2 = (r2 + g2 + b2) / 3;
            mean1 += gray1;
            mean2 += gray2;
            var1 += pow(gray1-mean1, 2);
            var2 += pow(gray2-mean2, 2);
            cov += (gray1-mean1) * (gray2-mean2);
        }
    }
    mean1 /= image1->h * image1->w;
    mean2 /= image1->h * image1->w;
    var1 /= image1->h * image1->w - 1;
    var2 /= image1->h * image1->w - 1;
    cov /= image1->h * image1->w - 1;
    //var1 -= mean1 * mean1;
    //var2 -= mean2 * mean2;
    //cov -= mean1 * mean2;
    // calculate luminance
    double luminance = (2 * mean1 * mean2 + C1) / (mean1 * mean1 + mean2 * mean2 + C1);
    // calculate contrast
    double contrast = (2 * sqrt(var1) * sqrt(var2) + C2) / (var1 + var2 + C2);
    // calculate structure
    double structure = (cov + C3) / (sqrt(var1) * sqrt(var2) + C3);
    // calculate ssim with weights
    //double ssim = 1 * luminance + 0 * contrast + 0 * structure;
    double ssim = (2 * mean1 * mean2 + C1) * (2 * cov + C2);
    ssim /= (mean1 * mean1 + mean2 * mean2 + C1) * (var1 + var2 + C2);
    printf("ssim: %f\n", ssim);
    return ssim;
}
/*double ssim(SDL_Surface* image1, SDL_Surface* image2) {
    double C1 = 0.01 * 255 * 0.01 * 255;
    double C2 = 0.03 * 255 * 0.03 * 255;
    double C3 = C2 / 2;
    double mean1 = 0;
    double mean2 = 0;
    double var1 = 0;
    double var2 = 0;
    double cov = 0;
    for (int i = 0; i < image1->h; i++) {
        for (int j = 0; j < image1->w; j++) {
            Uint32 pixel1 = get_pixel(image1, j, i);
            Uint32 pixel2 = get_pixel(image2, j, i);
            Uint8 r1, g1, b1;
            Uint8 r2, g2, b2;
            SDL_GetRGB(pixel1, image1->format, &r1, &g1, &b1);
            SDL_GetRGB(pixel2, image2->format, &r2, &g2, &b2);
            //double gray1 = 0.2126 * r1 + 0.7152 * g1 + 0.0722 * b1;
            //double gray2 = 0.2126 * r2 + 0.7152 * g2 + 0.0722 * b2;
            double gray1 = (r1 + g1 + b1) / 3;
            double gray2 = (r2 + g2 + b2) / 3;
            mean1 += gray1;
            mean2 += gray2;
            var1 += gray1 * gray1;
            var2 += gray2 * gray2;
            cov += gray1 * gray2;
        }
    }
    mean1 /= image1->h * image1->w;
    mean2 /= image1->h * image1->w;
    var1 /= image1->h * image1->w;
    var2 /= image1->h * image1->w;
    cov /= image1->h * image1->w;
    var1 -= mean1 * mean1;
    var2 -= mean2 * mean2;
    cov -= mean1 * mean2;
    double ssim = ((2 * mean1 * mean2 + C1) * (2 * cov + C2)) / ((mean1 * mean1 + mean2 * mean2 + C1 ) * (var1 + var2 + C2));
    return ssim;
}*/

// test the ssim function
/*int main(int argc, char* argv[]) {
    if (argc != 3) {
        errx(EXIT_FAILURE, "Usage: %s image1 image2", argv[0]);
    }
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDL_Surface* image1 = read_image(argv[1]);
    SDL_Surface* image2 = read_image(argv[2]);
    SDL_Surface* gray1 = convert_to_grayscale(image1);
    SDL_Surface* gray2 = convert_to_grayscale(image2);
    double ssim_value = ssim(gray1, gray2);
    printf("ssim: %f (the higher the better) \n ", ssim_value);
    SDL_FreeSurface(image1);
    SDL_FreeSurface(image2);
    SDL_FreeSurface(gray1);
    SDL_FreeSurface(gray2);
    IMG_Quit();
    SDL_Quit();
    return 0;
}*/