#include "pretreatment.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static Uint32 get_pixel(const SDL_Surface* surf, int x, int y)
{
    Uint8* p = (Uint8*)surf->pixels + y * surf->pitch + x * 4;
    return *(Uint32*)p;
}

static void set_pixel(SDL_Surface* surf, int x, int y, Uint32 color)
{
    Uint8* p    = (Uint8*)surf->pixels + y * surf->pitch + x * 4;
    *(Uint32*)p = color;
}

SDL_Surface* rotate_image(const SDL_Surface* src, double angle)
{
    if (!src)
        return NULL;

    double rad   = angle * M_PI / 180.0;
    double cos_a = cos(rad);
    double sin_a = sin(rad);

    int w = src->w;
    int h = src->h;

    int new_w = (int)(fabs(w * cos_a) + fabs(h * sin_a));
    int new_h = (int)(fabs(w * sin_a) + fabs(h * cos_a));

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, new_w, new_h, 32,
                                                      src->format->format);
    if (!dst)
        return NULL;

    if (SDL_MUSTLOCK(src))
        SDL_LockSurface((SDL_Surface*)src);
    if (SDL_MUSTLOCK(dst))
        SDL_LockSurface(dst);

    int cx  = w / 2;
    int cy  = h / 2;
    int ncx = new_w / 2;
    int ncy = new_h / 2;

    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            double tx = x - ncx;
            double ty = y - ncy;
            int    sx = (int)(+tx * cos_a + ty * sin_a + cx);
            int    sy = (int)(-tx * sin_a + ty * cos_a + cy);

            Uint32 color = 0xFFFFFFFF;
            if (sx >= 0 && sx < w && sy >= 0 && sy < h) {
                color = get_pixel(src, sx, sy);
            }
            set_pixel(dst, x, y, color);
        }
    }

    if (SDL_MUSTLOCK(src))
        SDL_UnlockSurface((SDL_Surface*)src);
    if (SDL_MUSTLOCK(dst))
        SDL_UnlockSurface(dst);

    return dst;
}
