#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
SDL_Surface *image;
const char *asciis = "  .:-=+*#%@";
// const char* asciis =
// "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";
// const char* asciis = "
// .'`^\",:;I!li<>~+-_?][}{1)(|/\\ftjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
// const char* asciis = " @%#*+=-:. ";

SDL_Surface *readImage(char *filename) {
  SDL_Surface *img = IMG_Load(filename);
  if (img == NULL)
    err(1, "Error: %s", SDL_GetError());
  return img;
}
void draw(SDL_Renderer *renderer, SDL_Texture *texture) {
  if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0)
    errx(EXIT_FAILURE, "%s", SDL_GetError());
  SDL_RenderPresent(renderer);
}

int get_average_brightness(SDL_Surface *cell) {
  int w = cell->w;
  int h = cell->h;
  Uint32 *pixels = (Uint32 *)cell->pixels;
  int sum = 0;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      Uint32 pixel = pixels[i * w + j];
      Uint8 r, g, b, a;
      SDL_GetRGBA(pixel, cell->format, &r, &g, &b, &a);
      sum += (r + g + b) / 3;
    }
  }
  return sum / (w * h);
}

int get_char(int brightness) { return brightness * strlen(asciis) / 255; }

char *asciify(SDL_Surface *image, int n, int m) {
  int w = image->w;
  int h = image->h;
  char *res = malloc(n * m * sizeof(char) + h);
  int k = 0;
  SDL_Surface *cell = SDL_CreateRGBSurfaceWithFormat(0, w / n, h / m, 32,
                                                     SDL_PIXELFORMAT_RGBA8888);
  //Uint32 *pixels = (Uint32 *)cell->pixels;
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      SDL_Rect rect = {j * h / m, i * w / n, w / n, h / m};
      /*if (i*w/n + w/n > w || j*h/m + h/m > h)
      {
          continue;
      }*/
      SDL_BlitSurface(image, &rect, cell, NULL);
      res[k] = asciis[get_char(get_average_brightness(cell))];
      k++;
    }
    res[k++] = '\n';
  }
  return res;
}
SDL_Surface *resize(SDL_Surface *image, int width, int height) {
  SDL_Surface *charsurface =
    SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
  SDL_BlitScaled(image, NULL, charsurface, NULL);
  return charsurface;
}
SDL_Surface *resizeFitTerminal(SDL_Surface *img) {
  // Resize the inital image to fit the terminal
  // Get the terminal size
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    errx(EXIT_FAILURE, "ioctl() failed");
  }
  return resize(img, w.ws_col, w.ws_row);
}
void processImage(SDL_Surface *img) {
  if (img == NULL)
    img = image;
  SDL_Surface *resized = resizeFitTerminal(img);
  char *ascii = asciify(resized, 1, 1);
  // Print the ASCII representation of the image to the terminal
  printf("%s\n", ascii);
  // Clean up
  SDL_FreeSurface(resized);
  free(ascii);
}
void onTerminalResize() {
  // Clear the terminal
  printf("\033[2J");
  // Move cursor to the top left
  printf("\033[%d;%dH", 0, 0);
  processImage(image);
  // Refresh the terminal window
  fflush(stdout);
}
// Register the signal handler for the terminal window resize event
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
// test
int main(int argc, char *argv[]) {
  if (argc != 3)
    errx(1, "Usage: %s <image> <cell size>", argv[0]);

  // read image
  image = readImage(argv[1]);
  image->format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

  // printf("%d %d\n", image->w, image->h);

  int n = image->w / atoi(argv[2]);
  int m = image->h / atoi(argv[2]);
  char *ascii = asciify(image, n, m);

  // print ascii
  printf("%s", ascii);
  registerResizeHandler();
  // Stay open until user presses a key
  getchar();
  // free memory
  SDL_FreeSurface(image);
  return 0;
}
