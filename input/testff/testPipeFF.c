#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <err.h>
#include <dirent.h>

 
// Video resolution
int W = 1280;
int H = 720;
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

void get_num_frames(char *filename, video_info *vi)
{
    int num_frames = 0;
    char command[200];
    char *pref = "ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 ";
    strcpy(command, pref);
    strcat(command, filename);
    FILE *pipein = popen(command, "r");
    fscanf(pipein, "%d", &num_frames);
    pclose(pipein);
    vi->num_frames = num_frames;
}

// get the video info with ffprobe
video_info get_video_info(char *filename)
{
    video_info info;
    char command[200];
    //char *pref = "ffprobe -v error -select_streams v:0 -show_entries stream=width,height,codec_name,r_frame_rate -of csv=s=x:p=0 ";

    char *pref1 = "ffprobe -v error -select_streams v:0 -show_entries stream=width -of csv=s=x:p=0 ";
    strcpy(command, pref1);
    strcat(command, filename);
    FILE *pipein = popen(command, "r");
    fscanf(pipein, "%d", &info.width);


    char *pref2 = "ffprobe -v error -select_streams v:0 -show_entries stream=height -of csv=s=x:p=0 ";
    strcpy(command, pref2);
    strcat(command, filename);
    pipein = popen(command, "r");
    fscanf(pipein, "%d", &info.height);

    char *pref3 = "ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of csv=s=x:p=0 ";
    strcpy(command, pref3);
    strcat(command, filename);
    pipein = popen(command, "r");
    fscanf(pipein, "%s", info.codec);

    char *pref4 = "ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate -of csv=s=x:p=0 ";
    strcpy(command, pref4);
    strcat(command, filename);
    pipein = popen(command, "r");
    fscanf(pipein, "%d", &info.fps);

    get_num_frames(filename, &info);
    pclose(pipein);
    return info;
}

void frame2ascii(unsigned char frame[H][W][3], char *asciis, int len, int tile_size)
{
    int i, j, x, y, sum = 0;
    char ascii_char;

    for (y = 0; y < H; y += tile_size)
    {
        for (x = 0; x < W; x += tile_size)
        {
            sum = 0;

            for (j = 0; j < tile_size; ++j)
            {
                for (i = 0; i < tile_size; ++i)
                {
                    if (y + j < H && x + i < W)
                    {
                        sum += (frame[y + j][x + i][0] + frame[y + j][x + i][1] + frame[y + j][x + i][2]);
                    }
                }
            }

            sum /= (3 * tile_size * tile_size);
            ascii_char = asciis[sum * len / 255];

            putchar(ascii_char);
        }

        putchar('\n');
    }
}

void read_video(char *filename, char *asciis, int len, int tile_size, int fps)
{
    int count;
    unsigned char frame[H][W][3];

    //char res[H/tile_size * (W/tile_size+1)];

    //char *post = " -f image2pipe -framerate 15 -vcodec h264 -pix_fmt rgb24 -";
    // make a string to hold the command
    char command[200];
    char *pref = "ffmpeg -i ";
    // copy the command into the string
    strcpy(command, pref);
    strcat(command, filename);
    strcat(command, " -f image2pipe -framerate 20");
    strcat(command, " -vcodec rawvideo -pix_fmt rgb24 -");

    // Open an input pipe from ffmpeg and an output pipe to a second instance of ffmpeg
    FILE *pipein = popen(command, "r");


    // Process video frames
    while(1)
    {
        //memset(res, 0, (H/tile_size + 1) * W/tile_size);

        // Read a frame from the input pipe into the buffer
        count = fread(frame, 1, H*W*3, pipein);

        // If we didn't get a frame of video, we're probably at the end
        if (count != H*W*3) break;

        system("clear");
        frame2ascii(frame, asciis, len, tile_size);
        //printf("%s", res);
        //k++;
        usleep(1000000/fps);
    }
    fflush(pipein);
    pclose(pipein);
}





int main(int argc, char *argv[])
{
    W = atoi(argv[4]);
    H = atoi(argv[5]);
    char *asciis = " .:-=+*#%@";
    //char *asciis = " .,:ilwW";
    int len = strlen(asciis);
    int tile_size = atoi(argv[2]);
    int fps = atoi(argv[3]);

    // get the number of frames with ffprobe



    read_video(argv[1], asciis, len, tile_size, fps);

    return EXIT_SUCCESS;
}
