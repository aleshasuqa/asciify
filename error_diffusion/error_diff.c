#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>

SDL_Surface* load_image(const char* path)
{
    SDL_Surface* src = IMG_Load(path);
    if (src == NULL)
	errx(EXIT_FAILURE, "%s", SDL_GetError());
    return src;
}

static int clamp(int value, int min, int max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

Uint32 get_pixel(SDL_Surface* surface, int x, int y) {
    // Get the number of bytes per pixel
    int bpp = surface->format->BytesPerPixel;
    
    // Get the pointer to the pixel at (x, y)
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    
    // Return the pixel value as a Uint32
    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *(Uint16*)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            } else {
                return p[0] | p[1] << 8 | p[2] << 16;
            }
        case 4:
            return *(Uint32*)p;
        default:
            return 0;
    }
}

void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
    // Get the number of bytes per pixel
    int bpp = surface->format->BytesPerPixel;
    
    // Get the pointer to the pixel at (x, y)
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    
    // Set the pixel value
    switch (bpp) {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(Uint16*)p = pixel;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xFF;
                p[1] = (pixel >> 8) & 0xFF;
                p[2] = pixel & 0xFF;
            } else {
                p[0] = pixel & 0xFF;
                p[1] = (pixel >> 8) & 0xFF;
                p[2] = (pixel >> 16) & 0xFF;
            }
            break;
        case 4:
            *(Uint32*)p = pixel;
            break;
    }
}

void Error_Diffusion(SDL_Surface* input, SDL_Surface* output) {
    // Define the error diffusion matrix
    const int matrix[3][3] = {
        { 0, 0, 7 },
        { 3, 5, 1 }
    };
    
    // Loop over each pixel in the input surface
    for (int y = 0; y < input->h; y++) {
        for (int x = 0; x < input->w; x++) {
            // Get the pixel at (x, y)
            Uint32 pixel = get_pixel(input, x, y);

            // Calculate the grayscale value of the pixel
            Uint8 r, g, b;
            SDL_GetRGB(pixel, input->format, &r, &g, &b);
	    /*
            Uint8 gray = (Uint8)(0.3 * r + 0.59 * g + 0.11 * b);
            
            // Set the output pixel to black or white based on the grayscale value
            if (gray < 128) {
                set_pixel(output, x, y, SDL_MapRGB(output->format, 0, 0, 0));
            } else {
                set_pixel(output, x, y, SDL_MapRGB(output->format, 255, 255, 255));
            }
            
            // Calculate the error
	    */
            int error = 128 - (get_pixel(output, x, y) & 0xFF);
            
            // Distribute the error to neighboring pixels
            for (int dy = 0; dy < 2; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    // Calculate the coordinates of the neighbor pixel
                    int nx = x + dx;
                    int ny = y + dy;
                    
                    // Skip the neighbor pixel if it's outside the image bounds
                    if (nx < 0 || nx >= input->w || ny < 0 || ny >= input->h) {
                        continue;
                    }
                    
                    // Distribute the error to the neighbor pixel
                    int factor = matrix[dy][dx + 1];
                    Uint32 neighbor = get_pixel(output, nx, ny);
                    SDL_GetRGB(neighbor, output->format, &r, &g, &b);
                    r = clamp(r + factor * error / 16, 0, 255);
                    g = clamp(g + factor * error / 16, 0, 255);
                    b = clamp(b + factor * error / 16, 0, 255);
                    set_pixel(output, nx, ny, SDL_MapRGB(output->format, r, g, b));
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc!=2)
        errx(EXIT_FAILURE,"please provide image file");

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "Error: SDL_Init failed: %s\n", SDL_GetError());

    SDL_Surface* input = load_image(argv[1]);

    // create output surface with same dimensions as input surface
    SDL_Surface* output = SDL_DuplicateSurface(input);
    if (output == NULL)
	    errx(EXIT_FAILURE, "%s", SDL_GetError());
    
    // lock input and output surfaces for pixel access
    SDL_LockSurface(input);
    SDL_LockSurface(output);

    Error_Diffusion(input,output);

    // Save the copySurface to the original file path
    if (IMG_SavePNG(output, "output.png") != 0) {
        printf("Error: could not save copy surface as PNG file: %s\n", IMG_GetError());
	
        SDL_FreeSurface(output);
        SDL_FreeSurface(input);
        SDL_Quit();
        return 1;
    }
    SDL_UnlockSurface(input);
    SDL_UnlockSurface(output);

    SDL_FreeSurface(output);
    SDL_FreeSurface(input);
}
