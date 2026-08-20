#include "pretreatment.h"

void convert_to_grayscale(SDL_Surface* surf)
{
    if (SDL_MUSTLOCK(surf))
        SDL_LockSurface(surf);

    Uint32* pixels = (Uint32*)surf->pixels;
    int     count  = (surf->w) * (surf->h);

    for (int i = 0; i < count; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], surf->format, &r, &g, &b, &a);

        Uint8 gray = (Uint8)(0.299 * r + 0.587 * g + 0.114 * b);

        pixels[i] = SDL_MapRGBA(surf->format, gray, gray, gray, a);
    }

    if (SDL_MUSTLOCK(surf))
        SDL_UnlockSurface(surf);
}
