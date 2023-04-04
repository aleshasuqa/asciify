#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Video resolution
int W = 1280;
int H = 720;
TTF_Font *SansFont;
SDL_Color white = {255, 255, 255, 255};
SDL_Renderer *renderer;

// Allocate a buffer to store one frame
// info about the video: width, height, number of frames, codec
typedef struct {
  int width;
  int height;
  int num_frames;
  int fps;
  char codec[20];
} video_info;

// get the number of frames with ffprobe
void get_num_frames(char *filename, video_info *vi) {
  int num_frames = 0;
  char command[200];
  char *pref =
    "ffprobe -v error -count_frames -select_streams v:0 -show_entries "
    "stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 ";
  strcpy(command, pref);
  strcat(command, filename);
  FILE *pipein = popen(command, "r");
  fscanf(pipein, "%d", &num_frames);
  pclose(pipein);
  vi->num_frames = num_frames;
}

// get the video info with ffprobe
video_info get_video_info(char *filename) {
  video_info info;
  char command[200];
  // char *pref = "ffprobe -v error -select_streams v:0 -show_entries
  // stream=width,height,codec_name,r_frame_rate -of csv=s=x:p=0 ";

  char *pref1 = "ffprobe -v error -select_streams v:0 -show_entries "
                "stream=width -of csv=s=x:p=0 ";
  strcpy(command, pref1);
  strcat(command, filename);
  FILE *pipein = popen(command, "r");
  fscanf(pipein, "%d", &info.width);

  char *pref2 = "ffprobe -v error -select_streams v:0 -show_entries "
                "stream=height -of csv=s=x:p=0 ";
  strcpy(command, pref2);
  strcat(command, filename);
  pipein = popen(command, "r");
  fscanf(pipein, "%d", &info.height);

  char *pref3 = "ffprobe -v error -select_streams v:0 -show_entries "
                "stream=codec_name -of csv=s=x:p=0 ";
  strcpy(command, pref3);
  strcat(command, filename);
  pipein = popen(command, "r");
  fscanf(pipein, "%s", info.codec);

  char *pref4 = "ffprobe -v error -select_streams v:0 -show_entries "
                "stream=r_frame_rate -of csv=s=x:p=0 ";
  strcpy(command, pref4);
  strcat(command, filename);
  pipein = popen(command, "r");
  fscanf(pipein, "%d", &info.fps);

  get_num_frames(filename, &info);
  pclose(pipein);
  return info;
}

void frame2ascii(unsigned char frame[H][W][3], char *asciis, int len,
                 int tile_size, char *output) {
  int i, j, x, y, sum = 0;
  char ascii_char;
  for (y = 0; y < H; y += tile_size) {
    for (x = 0; x < W; x += tile_size) {
      sum = 0;

      for (j = 0; j < tile_size; ++j) {
        for (i = 0; i < tile_size; ++i) {
          if (y + j < H && x + i < W) {
            sum += (frame[y + j][x + i][0] + frame[y + j][x + i][1] +
                    frame[y + j][x + i][2]);
          }
        }
      }

      sum /= (3 * tile_size * tile_size);
      ascii_char = asciis[sum * len / 255];

      *(output++) = ascii_char;
      // putchar(ascii_char);
    }
    *(output++) = '\n';
    // putchar('\n');
  }
}

void read_video(char *filename, char *asciis, int len, int tile_size, int fps) {
  int count;
  unsigned char frame[H][W][3];

  // char res[H/tile_size * (W/tile_size+1)];

  // char *post = " -f image2pipe -framerate 15 -vcodec h264 -pix_fmt rgb24 -";
  //  make a string to hold the command
  char command[200];
  char *pref = "ffmpeg -i ";
  // copy the command into the string
  strcpy(command, pref);
  strcat(command, filename);
  strcat(command, " -f image2pipe -framerate 20");
  strcat(command, " -vcodec rawvideo -pix_fmt rgb24 -");

  // Open an input pipe from ffmpeg and an output pipe to a second instance of
  // ffmpeg
  FILE *pipein = popen(command, "r");

  char *output = malloc(H / tile_size * (W/tile_size + 1));
  if (output == NULL)
    errx(EXIT_FAILURE, "Failed to malloc output");
  int line_length = W/tile_size + 1;
  char* out = output;
  // Process video frames
  while (1) {
    // memset(res, 0, (H/tile_size + 1) * W/tile_size);

    // Read a frame from the input pipe into the buffer
    count = fread(frame, 1, H * W * 3, pipein);

    // If we didn't get a frame of video, we're probably at the end
    if (count != H * W * 3)
      break;

    frame2ascii(frame, asciis, len, tile_size, output);
    /*SDL_Rect lineRect;
    lineRect.x = 0;
    lineRect.y = 0;
    lineRect.w = 100;
    lineRect.h = 100;
    */
    SDL_RenderClear(renderer);
    char *line = NULL;
    line = memcpy(line, out, line_length);
    for(int i=0;i<line_length; i++)
      putchar(line[i]);

    SDL_Surface *outSurface = TTF_RenderText_Solid(SansFont, line, white);
    if (outSurface == NULL)
      errx(EXIT_FAILURE, "Text surface failed");
    SDL_Texture *outTexture =
      SDL_CreateTextureFromSurface(renderer, outSurface);
    if (outTexture == NULL)
      errx(EXIT_FAILURE, "Text texture failed");

    SDL_Rect rect = {0, 0, outSurface->w, outSurface->h};
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // set color to black
    // SDL_RenderFillRect(renderer, &rect); // fill window with black color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // set color to white
    if (SDL_RenderCopy(renderer, outTexture, NULL, &rect) != 0)
      errx(EXIT_FAILURE, "Rendering failed");

    SDL_RenderPresent(renderer);

    SDL_FreeSurface(outSurface);
    SDL_DestroyTexture(outTexture);
    usleep(1000000 / fps);
    out += line_length;
  }

  free(output);
  fflush(pipein);
  pclose(pipein);
}

void draw(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  SDL_RenderPresent(renderer);
}

void event_loop(SDL_Renderer *renderer) {
  // draw(renderer);

  SDL_Event event;

  while (1) {
    // Waits for an event.
    SDL_WaitEvent(&event);

    switch (event.type) {
    // If the "quit" button is pushed, ends the event loop.
    case SDL_QUIT:
      return;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 6) {
    printf("%s <input_source> <tile_size> <fps> <width> <height>\n", argv[0]);
    return EXIT_FAILURE;
  }
  // Initializes the SDL.
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    errx(EXIT_FAILURE, "%s", SDL_GetError());
  if (TTF_Init() != 0)
    errx(EXIT_FAILURE, "Failed to init TTF");
  // Creates a window.
  SDL_Window *window = SDL_CreateWindow(
    "ASCII-fy", 0, 0, W, H, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (window == NULL)
    errx(EXIT_FAILURE, "%s", SDL_GetError());

  // Creates a renderer.
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL)
    errx(EXIT_FAILURE, "%s", SDL_GetError());
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
  //printf("background should be black\n");

  SansFont = TTF_OpenFont("Sans.ttf", 24);
  if (SansFont == NULL) {
    printf("TTF_OpenFont Error: %s\n", TTF_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  /*white.r = 255;
  white.g = 255;
  white.b = 255;*/

  W = atoi(argv[4]);
  H = atoi(argv[5]);
  char *asciis = " .:-=+*#%@";
  // char *asciis = " .,:ilwW";
  int len = strlen(asciis);
  int tile_size = atoi(argv[2]);
  int fps = atoi(argv[3]);

  // get the number of frames with ffprobe
  read_video(argv[1], asciis, len, tile_size, fps);

  event_loop(renderer);
  // Destroys the objects.
  TTF_CloseFont(SansFont);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
