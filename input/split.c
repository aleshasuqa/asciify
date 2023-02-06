#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
const char* asciis = "  .:-=+*#%@";
//const char* asciis = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";
//const char* asciis = "            .'`^\",:;I!li<>~+-_?][}{1)(|/\\ftjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
//const char* asciis = " @%#*+=-:. ";

void draw(SDL_Renderer* renderer, SDL_Texture* texture)
{
    if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    SDL_RenderPresent(renderer);
}

void event_loop(SDL_Renderer* renderer, SDL_Texture* colored)
{
    SDL_Event event;
    SDL_Texture* t = colored;
    draw(renderer, t);

    while (1)
    {
        SDL_WaitEvent(&event);

        switch (event.type)
        {
            // If the "quit" button is pushed, ends the event loop.
            case SDL_QUIT:
                return;

                // If the window is resized, updates and redraws the diagonals.
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    draw(renderer, t);
                }
                break;
        }
    }
}

int get_average_brightness(SDL_Surface *cell) {
    int w = cell->w;
    int h = cell->h;
    Uint32 *pixels = (Uint32*)cell->pixels;
    int sum = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Uint32 pixel = pixels[i*w+j];
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, cell->format, &r, &g, &b, &a);
            sum += (r+g+b)/3;
        }
    }
    return sum/(w*h);
}

int get_char(int brightness) {
    return brightness*strlen(asciis)/255;
}

char* asciify(SDL_Surface *image, int n, int m) {
    int w = image->w;
    int h = image->h;
    char *res = malloc(n*m*sizeof(char)+h);
    int k = 0;
    SDL_Surface *cell = SDL_CreateRGBSurfaceWithFormat(0, w/n, h/m, 32, SDL_PIXELFORMAT_RGBA8888);
    Uint32 *pixels = (Uint32*)cell->pixels;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            SDL_Rect rect = {j*h/m, i*w/n, w/n, h/m};
            if (i*w/n + w/n > w || j*h/m + h/m > h)
            {
                continue;
            }
            SDL_BlitSurface(image, &rect, cell, NULL);
            res[k] = asciis[get_char(get_average_brightness(cell))];
            k++;
        }
        res[k++] = '\n';
    }
    return res;
}

// test
int main(int argc, char *argv[])
{
    // read image
    SDL_Surface* image = IMG_Load(argv[1]);
    if (image == NULL) {
        printf("Error: %s\n", IMG_GetError());
        return 1;
    }
    image->format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    printf("%d %d\n", image->w, image->h);


    int n = image->w/atoi(argv[2]);
    int m = image->h/atoi(argv[2]);
    char *ascii = asciify(image, n, m);

    // write ascii to file
    /*FILE *f = fopen("ascii.txt", "w");
    fprintf(f, "%s", ascii);
    fclose(f);*/

    // print ascii
    printf("%s", ascii);

    // free memory
    SDL_FreeSurface(image);
    free(ascii);

    return 0;
}



