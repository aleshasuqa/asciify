#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <err.h>
#include <dirent.h>
#include <pthread.h>

 
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

    // fps
    char *pref3 = "ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate -of csv=s=x:p=0 ";
    strcpy(command, pref3);
    strcat(command, filename);
    pipein = popen(command, "r");
    fscanf(pipein, "%d", &info.fps);

    pclose(pipein);
    return info;
}

void flip_frame_horizontally(unsigned char frame[H][W][3])
{
    int i, j;
    unsigned char temp;
    for (j = 0; j < H; ++j)
    {
        for (i = 0; i < W/2; ++i)
        {
            temp = frame[j][i][0];
            frame[j][i][0] = frame[j][W-i-1][0];
            frame[j][W-i-1][0] = temp;

            temp = frame[j][i][1];
            frame[j][i][1] = frame[j][W-i-1][1];
            frame[j][W-i-1][1] = temp;

            temp = frame[j][i][2];
            frame[j][i][2] = frame[j][W-i-1][2];
            frame[j][W-i-1][2] = temp;
        }
    }
}

void frame2ascii(unsigned char frame[H][W][3], const char *asciis, const int len, const int tile_size)
{
    int i, j, x, y, sum = 0;
    for (y=0 ; y<H ; y += tile_size)
    {
        for (x=0 ; x<W ; x+=tile_size)
        {
            for (j = 0; j<tile_size; ++j) for (i = 0; i<tile_size; ++i)
            {
                if (y+j < H && x+i < W)
                {
                    sum += (frame[y+j][x+i][0] + frame[y+j][x+i][1] + frame[y+j][x+i][2])/3;
                }
            }
            sum /= tile_size*tile_size;
            printf("%c", asciis[sum*len/255]);
        }
        printf("\n");
    }
}

void frame2ascii2(unsigned char frame[H][W][3], const char *asciis, const int len, const int tile_size)
{
    int i, j, x, y, sum;
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

void frame2ascii2_flip_horizontally(unsigned char frame[H][W][3], const char *asciis, const int len, const int tile_size)
{
    int i, j, x, y, sum;
    char ascii_char;

    for (y = 0; y < H; y += tile_size)
    {
        for (x = W; x > 0; x -= tile_size)
        {
            sum = 0;

            for (j = 0; j < tile_size; ++j)
            {
                for (i = 0; i < tile_size; ++i)
                {
                    if (y + j < H && x - i > 0)
                    {
                        sum += (frame[y + j][x - i][0] + frame[y + j][x - i][1] + frame[y + j][x - i][2]);
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


void read_video(char cmd[1024], const char *asciis, const int len, const int tile_size, int fps)
{
    int count;
    unsigned char frame[H][W][3];
    //printf("%s", cmd);

    //har cmd[1024];
    //snprintf(cmd, sizeof(cmd), "ffmpeg -f %s -f image2pipe -pix_fmt rgb24 -vcodec rawvideo -", filename);

    // Open an input pipe from ffmpeg and an output pipe to a second instance of ffmpeg
    FILE *pipein = popen(cmd, "r");

    for (int i = 0; i < 50; i++)
        putchar('\n');


    // Process video frames
    while(1)
    {

        // Read a frame from the input pipe into the buffer
        count = fread(frame, 1, H*W*3, pipein);

        // If we didn't get a frame of video, we're probably at the end
        if (count != H*W*3) break;

        // clear the screen
        printf("\033[2J\033[1;1H");
        frame2ascii2(frame, asciis, len, tile_size);
        usleep(1000000/fps);
    }
    fflush(pipein);
    pclose(pipein);
}

void read_video_flip(char cmd[1024], const char *asciis, const int len, const int tile_size, int fps)
{
    int count;
    unsigned char frame[H][W][3];
    //printf("%s", cmd);

    //har cmd[1024];
    //snprintf(cmd, sizeof(cmd), "ffmpeg -f %s -f image2pipe -pix_fmt rgb24 -vcodec rawvideo -", filename);

    // Open an input pipe from ffmpeg and an output pipe to a second instance of ffmpeg
    FILE *pipein = popen(cmd, "r");


    // Process video frames
    while(1)
    {

        // Read a frame from the input pipe into the buffer
        count = fread(frame, 1, H*W*3, pipein);

        // If we didn't get a frame of video, we're probably at the end
        if (count != H*W*3) break;

        // clear the screen
        printf("\033[2J\033[1;1H");
        frame2ascii2_flip_horizontally(frame, asciis, len, tile_size);
        usleep(1000000/fps);
    }
    fflush(pipein);
    pclose(pipein);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage:\nVideo: <video_file> <tile_size> <fps>\nWebcam: web_cam <tile_size>\nPhone cam: phone_cam <tile_size>\n");
        return 0;
    }
    system("clear");
    char cmd[1024];
    const char *asciis = " .,:-=+*#%@";
    const int len = strlen(asciis);
    const int tile_size = atoi(argv[2]);
    int fps = 60;
    #if __APPLE__
    {
        if (strcmp(argv[1], "web_cam") == 0)
        {
            system("printf '\e]50;SetProfile=cam\x7'");
            snprintf(cmd, sizeof(cmd), "ffmpeg -f avfoundation -video_size 1280x720 -framerate 30 -i \"0\" -f image2pipe -pix_fmt rgb24 -vcodec rawvideo -");
            //printf("%s\n\n\n", cmd);
            H = 720;
            W = 1280;
            char resize[100];
            snprintf(resize, sizeof(resize), "printf '\e[8;%i;%it'", H/tile_size, W/tile_size);
            system(resize);
            read_video_flip(cmd, asciis, len, tile_size, fps);
        }
        else if (strcmp(argv[1], "phone_cam") == 0)
        {
            system("printf '\e]50;SetProfile=cam\x7'");
            snprintf(cmd, sizeof(cmd), "ffmpeg -f avfoundation -video_size 1280x720 -framerate 30 -i \"1\" -f image2pipe -pix_fmt rgb24 -vcodec rawvideo -");
            H = 720;
            W = 1280;
            
            char resize[100];
            snprintf(resize, sizeof(resize), "printf '\e[8;%i;%it'", H/tile_size, W/tile_size);
            system(resize);
            read_video(cmd, asciis, len, tile_size, fps);
        }
        else
        {
            system("printf '\e]50;SetProfile=vid\x7'");
            video_info info = get_video_info(argv[1]);
            H = info.height;
            W = info.width;
            char resize[100];
            snprintf(resize, sizeof(resize), "printf '\e[8;%i;%it'", H/tile_size, W/tile_size);
            fps = atoi(argv[3]);
            snprintf(cmd, sizeof(cmd), "ffmpeg -i %s -f image2pipe -pix_fmt rgb24 -vcodec rawvideo -", argv[1]);
            system(resize);
            read_video(cmd, asciis, len, tile_size, fps);
        }
    }
    #elif _WIN32
        // windows specific code
    #elif __unix__
    {
        if (strcmp(argv[1], "web_cam") == 0)
        {
            snprintf(cmd, sizeof(cmd), "ffmpeg -i /dev/video0 -f image2pipe -pix_fmt rgb24 -vcodec rawvideo -");
            //printf("%s\n\n\n", cmd);
            H = 720;
            W = 1280;
            read_video(cmd, asciis, len, tile_size, fps);
        }
        else
        {
            video_info info = get_video_info(argv[1]);
            H = info.height;
            W = info.width;
            fps = atoi(argv[3]);
            snprintf(cmd, sizeof(cmd), "ffmpeg -i %s -f image2pipe -pix_fmt rgb24 -vcodec rawvideo -", argv[1]);
            read_video(cmd, asciis, len, tile_size, fps);
        }
        // linux specific code
    }
    #elif BSD
        // BSD specific code
    #else
        // general code or warning
    #endif

    //W = atoi(argv[4]);
    //H = atoi(argv[5]);

    //read_video(argv[1], asciis, len, tile_size, fps);

    return EXIT_SUCCESS;
}
