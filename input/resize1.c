#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
SDL_Surface *image;

SDL_Surface *resizeFitTerminal(SDL_Surface *img) {
   // Resize the inital image to fit the terminal
   // Get the terminal size
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        errx(EXIT_FAILURE, "ioctl() failed");
    }
    //resize the image, but KEEP the aspect ratio of the original image
    int newWidth = (img->w * w.ws_row) / img->h;
    int newHeight = (img->h * w.ws_col) / img->w;
    SDL_Surface *resized = SDL_CreateRGBSurface(0, newWidth, newHeight, 32, 0, 0, 0, 0);

    if (SDL_BlitScaled(img, NULL, resized, NULL) != 0) {
        errx(EXIT_FAILURE, "SDL_BlitScaled() failed");
    }
    //save resized to file, filename contains width and height
    char filename[100];
    sprintf(filename, "resized_%dx%d.bmp", w.ws_col, w.ws_row);
    if (SDL_SaveBMP(resized, filename) != 0) {
        errx(EXIT_FAILURE, "SDL_SaveBMP() failed");
    }
    return resized;
}
void onTerminalResize() {
  SDL_Surface *resized = resizeFitTerminal(image);
  SDL_FreeSurface(resized);
}
void registerResizeHandler() {
  struct sigaction sa;

  sa.sa_handler = onTerminalResize;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGWINCH, &sa, NULL) == -1) {
    perror("sigaction resize");
    exit(1);
  }
}
int main(int argc, char *argv[]){
    if (argc != 2) {
        errx(EXIT_FAILURE, "Usage: %s <image>", argv[0]);
    }
    
    SDL_Surface *img = IMG_Load(argv[1]);
    if (img == NULL) {
        errx(EXIT_FAILURE, "IMG_Load() failed");
    }
    image = img;
    SDL_Surface *resized = resizeFitTerminal(img);
    registerResizeHandler();
    // Stay open until user presses a key
    getchar();
    SDL_FreeSurface(img);
    SDL_FreeSurface(resized);
    return EXIT_SUCCESS;
}