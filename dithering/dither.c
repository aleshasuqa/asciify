#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

void draw(SDL_Renderer* renderer, SDL_Texture* texture)
{
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void event_loop(SDL_Renderer* renderer, SDL_Texture* dithered) //SDL_Texture* colored, SDL_Texture* grayscale, SDL_Texture* dithered)
{
    SDL_Event event;
    SDL_Texture* texture = dithered; //colored;

    while (1)
    {
        SDL_WaitEvent(&event);

        switch (event.type)
        {
		case SDL_QUIT:
			return;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				draw(renderer, texture);
			}
			break;
	//	case SDL_KEYDOWN:
	//		if(texture == colored)
	//			texture = grayscale;
	//		if(texture == grayscale)
	//			texture = dithered;
	//		else
	//			texture = colored;
	//		draw(renderer, texture);
	//		break;
        }
    }
}

SDL_Surface* load_image(const char* path)
{
    SDL_Surface* src = IMG_Load(path);
    if (src == NULL)
	errx(EXIT_FAILURE, "%s", SDL_GetError());
    SDL_Surface* surface = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB888, 0);
    if (surface == NULL)
	errx(EXIT_FAILURE, "%s", SDL_GetError());
    SDL_FreeSurface(src);
    return surface;
}

Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);
    Uint8 average = 0.3*r + 0.59*g + 0.11*b;
    Uint32 color = SDL_MapRGB(format, average, average, average);
    return color;
}

void surface_to_grayscale(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;
    int locked = SDL_LockSurface(surface);
    if (locked != 0)
	    errx(EXIT_FAILURE, "%s", SDL_GetError());
    for(int i = 0; i < len; i++)
            pixels[i] =  pixel_to_grayscale(pixels[i], format);
    SDL_UnlockSurface(surface);
}

 
void surface_dither(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    int width = surface->w;
    int height = surface->h;
    //SDL_PixelFormat* format = surface->format;
    int locked = SDL_LockSurface(surface);
    if (locked != 0)
	    errx(EXIT_FAILURE, "%s", SDL_GetError());
    //Uint32 black = SDL_MapRGB(format, 0, 0, 0);
    //Uint32 white = SDL_MapRGB(format, 255, 255, 255);
    for(int j = 1; j < height - 1; j++){
	 for(int i = 0; i < width - 1; i++){
		 //Uint8 r, g, b;
		 //SDL_GetRGB(pixels[j * width + i], format, &r, &g, &b);
	       	 //Uint32 quant_pixel = r > 127 ? white : black;
		 Uint32 quant_pixel = pixels[j * width + i] > 127 ? 255 : 0;
		 Uint32 quant_error = pixels[j * width + i] - quant_pixel;
		 //pixels[j * width +i] = quant_pixel;
		 pixels[(j + 1) * width + i] =  pixels[(j + 1) * width + i] + quant_error * 7/16;
		 pixels[(j - 1) * width + (i + 1)] = pixels[(j-1) * width + (i+1)] + quant_error*3/16;
                 pixels[j * width + (i + 1)] = pixels[j * width + (i + 1)] +quant_error * 5/16;
         	 pixels[(j + 1) * width + (i + 1)] = pixels[(j+1) * width + (i+1)] + quant_error*1/16;
	 	}
	 }

    SDL_UnlockSurface(surface);
}

int main(int argc, char** argv)
{
    if (argc != 2)
        errx(EXIT_FAILURE, "Usage: image-file");
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_Window* window = SDL_CreateWindow("Dithering result", 0, 0, 640, 400,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_Surface* surface = load_image(argv[1]);


    SDL_SetWindowSize(window, surface->w, surface->h);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    surface_to_grayscale(surface);
    SDL_Texture* gray_texture = SDL_CreateTextureFromSurface(renderer, surface);
    surface_dither(surface);
    SDL_Texture* dithered_texture = SDL_CreateTextureFromSurface(renderer, surface);
    surface_to_grayscale(surface);//
    SDL_Texture* double_gray_texture = SDL_CreateTextureFromSurface(renderer, surface);//
    SDL_FreeSurface(surface);
    event_loop(renderer, double_gray_texture); //texture, gray_texture, dithered_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(gray_texture);
    SDL_DestroyTexture(dithered_texture);
    SDL_DestroyTexture(double_gray_texture);//
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
