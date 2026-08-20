#include "pretreatment.h"

SDL_Surface* enhance_contrast(const SDL_Surface* surf)
{
    if (!surf)
        return NULL;

    SDL_Surface* out = SDL_CreateRGBSurfaceWithFormat(0, surf->w, surf->h, 32,
                                                      surf->format->format);
    if (!out)
        return NULL;

    if (SDL_MUSTLOCK(surf))
        SDL_LockSurface((SDL_Surface*)surf);
    if (SDL_MUSTLOCK(out))
        SDL_LockSurface(out);

    Uint32* in_pixels  = (Uint32*)surf->pixels;
    Uint32* out_pixels = (Uint32*)out->pixels;

    int w = surf->w;
    int h = surf->h;

    Uint8 min = 255, max = 0;

    for (int i = 0; i < w * h; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(in_pixels[i], surf->format, &r, &g, &b, &a);
        if (r < min)
            min = r;
        if (r > max)
            max = r;
    }

    double scale = (max > min) ? (255.0 / (max - min)) : 1.0;

    for (int i = 0; i < w * h; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(in_pixels[i], surf->format, &r, &g, &b, &a);
        Uint8 v       = (Uint8)((r - min) * scale);
        out_pixels[i] = SDL_MapRGBA(out->format, v, v, v, 255);
    }

    if (SDL_MUSTLOCK(surf))
        SDL_UnlockSurface((SDL_Surface*)surf);
    if (SDL_MUSTLOCK(out))
        SDL_UnlockSurface(out);

    return out;
}
