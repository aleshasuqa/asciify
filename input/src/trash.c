/*int w = cell->w;
    int h = cell->h;
    Uint32 *pixels = (Uint32*)cell->pixels;
    double sum = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Uint32 pixel = pixels[i*w+j];
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, cell->format, &r, &g, &b, &a);
            sum += (r+g+b)/3;
        }
    }
    return sum/(w*h);*/

/*int w = cell->w;
    int h = cell->h;
    double sum = 0;
    Uint32 *pixels = (Uint32*)cell->pixels;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            Uint32 pixel = pixels[i*w+j];
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, cell->format, &r, &g, &b, &a);
            sum += pow((((r+g+b)/3)-mu), 2);
        }
    }
    return sqrt(sum/(w*h-1));*/


double get_structure(SDL_Surface *cellx, SDL_Surface *celly, double mux, double muy)
{
    int w = cellx->w;
    int h = cellx->h;
    Uint32 *pixelsx = (Uint32*)cellx->pixels;
    Uint32 *pixelsy = (Uint32*)celly->pixels;
    double sum = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Uint32 pixelx = pixelsx[i*w+j];
            Uint32 pixely = pixelsy[i*w+j];
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixelx, cellx->format, &r, &g, &b, &a);
            double xi = (r+g+b)/3;
            SDL_GetRGBA(pixely, celly->format, &r, &g, &b, &a);
            double yi = (r+g+b)/3;
            sum += (xi-mux)*(yi-muy);
        }
    }
    return sum/(w*h-1);
}

double calcLC(double x, double y)
{
    if (x == 0 || y == 0)
        return 1;
    return (2*x*y)/(pow(x, 2)+pow(y, 2));
}

double calcS(double x, double y, double cor)
{
    if (x == 0 || y == 0)
        return 1;
    return (pow(cor, 2))/(x*y);
}
