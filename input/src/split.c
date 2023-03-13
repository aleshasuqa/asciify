#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include "ssim.h"
#include <SDL2/SDL_ttf.h>
#include <dirent.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
//char* asciis = "  .:-=+*#%@";
//char asciis[100] = {0};
//const char* asciis = "ABCDEFGHIJKLMNOPQRSTUVWXYZa&ˆ˜*@b`|{}[]c:,d$e8=!f54g£>h-ijkl<mn9#o1p()%.+q?\"\'rs;76/ t32u";
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


// scale image to x*y
SDL_Surface* __scale(SDL_Surface* image, int x, int y) {
    SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(0, x, y, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_Rect rect = {0, 0, x, y};
    SDL_BlitScaled(image, NULL, scaled, &rect);
    return scaled;
}

// cut first n lines of an image
SDL_Surface* cut(SDL_Surface* image, int n) {
    SDL_Surface* cutted = SDL_CreateRGBSurfaceWithFormat(0, image->w, image->h-n, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_Rect rect = {0, n, image->w, image->h-n};
    SDL_BlitSurface(image, &rect, cutted, NULL);
    return cutted;
}



int get_average_brightness(SDL_Surface *cell) {
    int w = cell->w;
    int h = cell->h;
    Uint32 *pixels = (Uint32*)cell->pixels;
    int sum = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Uint32 pixel = pixels[i*w+j];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, cell->format, &r, &g, &b);
            sum += (r+g+b)/3;
        }
    }
    return sum/(w*h);
}

int get_char(int brightness, char *asciis) {
    return brightness*strlen(asciis)/255;
}

//scale image to x*y
SDL_Surface* scale(SDL_Surface* image, int x, int y) {
    SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(0, x, y, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_Rect rect = {0, 0, x, y};
    SDL_BlitScaled(image, NULL, scaled, &rect);
    return scaled;
}

//creatw an array of images of every character
SDL_Surface** create_chars(char *asciis) {
    SDL_Surface** chars = malloc(strlen(asciis)*sizeof(SDL_Surface*));
    int i = 0;
    DIR *d;
    struct dirent *dir;
    char *path = malloc(100);
    for (int k = 0; k < 2; k++)
    {
        if (k == 0)
            path = "../glyphs/glyphs1";
        else
            path = "../glyphs/glyphs2";
        d = opendir(path);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                printf("%s\n", dir->d_name);
                char name;
                if (dir->d_name == "colon.png")
                    name = ':';
                else if (dir->d_name == "period.png")
                    name = '.';
                else if (dir->d_name[0] == '.')
                    continue;
                else
                    name = dir->d_name[0];
                asciis[i] = name;
                char *np = malloc(100);
                strcat(np, path);
                strcat(np, "/");
                strcat(np, dir->d_name);
                //printf("%s\n", np);
                SDL_Surface* glyph = IMG_Load(np);
                if (glyph == NULL) {
                    printf("Error: %s\n", IMG_GetError());
                    return NULL;
                }
                glyph->format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
                chars[i] = glyph;
                i++;
                asciis[i] = '\0';
                //printf("%c\n", name);
                free(np);
            }
            closedir(d);
        }
    }
    // free memory
    return chars;
}

SDL_Surface** get_frames(char *path, int *n) {
    int k = 0;
    struct dirent **namelist;
    *n = scandir(path, &namelist, NULL, alphasort);
    SDL_Surface** frames;
    if (*n < 0)
        perror("scandir");
    else {
        frames = malloc((*n-k) * sizeof(SDL_Surface*));
        for (int i = 0; i < *n; ++i) {
            if (namelist[i]->d_name[0] == '.') {
                k++;
                continue;
            }
            printf("%s\n", namelist[i]->d_name);
            char *buf = malloc(100);
            strcpy(buf, path);
            strcat(buf, "/");
            strcat(buf, namelist[i]->d_name);
            //strcat(buf, "\0");
            printf("%s\n", buf);
            frames[i-k] = IMG_Load(buf);
            //IMG_SavePNG(frames[i], "test.png");
            if (frames[i-k] == NULL) {
                errx(1, "IMG_Load: %s", IMG_GetError());
            }
            free(namelist[i]);
            free(buf);
        }
        for (int i = 0; i < k; ++i) {
            free(namelist[i]);
        }
        *n -= k;
        free(namelist);
    }
    //IMG_SavePNG(frames[0], "test.png");
    return frames;
}

//convert image to ascii by splitting the image into tiles and comparing the ssim index of a tile with an images of a character
char* asciify1(SDL_Surface *image, SDL_Surface** chars, char *asciis) {
    int w = image->w;
    int h = image->h;
    char *res = malloc(w/8 * h/8 + h);
    int k = 0;
    SDL_Surface *cell = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);
    for (int i = 0; i < h; i+=8) {
        for (int j = 0; j < w; j+=8) {
            SDL_Rect rect = {j, i, 8, 8};

            //if (j + 8 > w || i + 8 > h)
            //{
            //    continue;
            //}
            SDL_BlitSurface(image, &rect, cell, NULL);

            double max = -1;
            int index = 0;
            for (int l = 0; l < strlen(asciis); l++) {
                printf("l: %d\n", l);
                double sim = ssim(cell, chars[l]);
                if (sim > max) {
                    max = sim;
                    index = l;
                }
            }
            printf("%c", asciis[index]);
            res[k] = asciis[index];
            k++;
        }
        res[k++] = '\n';
    }
    //res[++k] = '\0';
    return res;
}

char* asciify(SDL_Surface *image, int n, int m, char *asciis) {
    int w = image->w;
    int h = image->h;
    char *res = malloc(w/n*h/m+h+1);
    int k = 0;
    SDL_Surface *cell = SDL_CreateRGBSurface(0, n, m, 32, 0, 0, 0, 0);
    Uint32 *pixels = (Uint32*)cell->pixels;
    for (int j = 0; j < h; j+=m) {
        for (int i = 0; i < w; i+=n) {
            SDL_Rect rect;
            rect.x = i;
            rect.y = j;
            rect.w = n;
            rect.h = m;
            SDL_BlitSurface(image, &rect, cell, NULL);
            res[k] = asciis[get_char(get_average_brightness(cell), asciis)];
            if (res[k] == '\0')
                res[k] = asciis[strlen(asciis)-1];
            k++;
        }
        res[k++] = '\n';
        printf("\n");
    }
    res[k] = '\0';
    SDL_FreeSurface(cell);
    return res;
}




// test
int main(int argc, char *argv[])
{
    if (argc == 3) {
        SDL_Surface *image = IMG_Load(argv[1]);
        int n = atoi(argv[2]);
        char *asciis = "  .:-=+*#%@";
        char *res = asciify(image, n, n, asciis);
        printf("%s", res);
        SDL_FreeSurface(image);
        free(res);
        return 0;
    }

    if (argc == 4)
    {
        int l = 0;
        SDL_Surface** frames = get_frames(argv[1], &l);
        //IMG_SavePNG(frames[0], "test.png");
        //SDL_Surface *image = IMG_Load(argv[1]);

        int n = atoi(argv[2]);
        int d = atoi(argv[3]);

        char *asciis = "  .:-=+*#%@";


        char **res = malloc(l*sizeof(char*));
        for (int i = 0; i < l; i++) {
            //printf("i: %i\n", i);
            //IMG_SavePNG(frames[i], "test.png");
            res[i] = asciify(frames[i], n, n, asciis);
            //printf("i: %i len: %i must: %i\n", i, strlen(res[i]), frames[i]->w/n*frames[i]->h/n+frames[i]->h);
        }

        for (int _ = 0; _ < 10; ++_) {
            for (int i = 0; i < l; ++i) {
                //clear screen
                printf("\033[2J\033[1;1H");
                printf("%s", res[i]);
                SDL_Delay(d);
            }
        }

        // clear screen

        // free memory


        // free memory
        for (int i = 0; i < l; ++i) {
            free(res[i]);
        }
        free(res);
        for (int i = 0; i < l; i++) {
            SDL_FreeSurface(frames[i]);
        }
        return 0;
    }

    if (argc == 5) {
        int l = 0;
        SDL_Surface** frames = get_frames(argv[1], &l);
        //IMG_SavePNG(frames[0], "test.png");
        //SDL_Surface *image = IMG_Load(argv[1]);

        int n = atoi(argv[2]);
        int d = atoi(argv[3]);

        char *asciis = "  .:-=+*#%@";

        for (int _ = 0; _ < 10; ++_) {
            for (int i = 0; i < l; ++i) {
                //clear screen
                printf("\033[2J\033[1;1H");
                char *res = asciify(frames[i], n, n, asciis);
                printf("%s", res);
                free(res);
                SDL_Delay(d);
            }
        }
        for (int i = 0; i < l; i++) {
            SDL_FreeSurface(frames[i]);
        }
        return 0;
    }

}
