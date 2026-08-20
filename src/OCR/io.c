#include "pretreatment.h"

SDL_Surface* load_image(const char* path)
{
    SDL_Surface* loaded = IMG_Load(path);
    if (!loaded) {
        fprintf(stderr, "IMG_Load error: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Surface* formatted =
        SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(loaded);
    return formatted;
}

int save_image(const char* path, SDL_Surface* surf)
{
    if (IMG_SavePNG(surf, path) != 0) {
        fprintf(stderr, "IMG_SavePNG error: %s\n", IMG_GetError());
        return -1;
    }
    return 0;
}
