#include <SDL2/SDL.h>
#include "image_splitter.h"

#define TARGET_SIZE 28

SDL_Surface* resize_surface_to_50x50(SDL_Surface* src)
{
    SDL_Surface* resized =
        SDL_CreateRGBSurface(0, TARGET_SIZE, TARGET_SIZE, 32, 0x00FF0000,
                             0x0000FF00, 0x000000FF, 0xFF000000);

    if (!resized)
        return NULL;

    Uint32* src_px = (Uint32*)src->pixels;
    Uint32* dst_px = (Uint32*)resized->pixels;

    int src_pitch = src->pitch / 4;
    int dst_pitch = resized->pitch / 4;

    float scale_x = (float)src->w / TARGET_SIZE;
    float scale_y = (float)src->h / TARGET_SIZE;

    for (int y = 0; y < TARGET_SIZE; y++) {
        for (int x = 0; x < TARGET_SIZE; x++) {
            int src_x = (int)(x * scale_x);
            int src_y = (int)(y * scale_y);

            Uint32 pixel              = src_px[src_y * src_pitch + src_x];
            dst_px[y * dst_pitch + x] = pixel;
        }
    }

    return resized;
}
