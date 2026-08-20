#include "pretreatment.h"

SDL_Surface* binarize_image(SDL_Surface* src)
{
    if (!src)
        return NULL;
    if (src->format->format != SDL_PIXELFORMAT_ARGB8888) {
        SDL_Log("Surface must be ARGB8888!");
        return NULL;
    }

    int w = src->w;
    int h = src->h;

    SDL_Surface* dst =
        SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!dst)
        return NULL;

    if (SDL_MUSTLOCK(src))
        SDL_LockSurface(src);
    if (SDL_MUSTLOCK(dst))
        SDL_LockSurface(dst);

    Uint32* pixels     = (Uint32*)src->pixels;
    Uint32* out_pixels = (Uint32*)dst->pixels;

    Uint32* tmp_pixels = (Uint32*)malloc(sizeof(Uint32) * w * h);
    if (!tmp_pixels) {
        SDL_Log("malloc failed");
        if (SDL_MUSTLOCK(src))
            SDL_UnlockSurface(src);
        if (SDL_MUSTLOCK(dst))
            SDL_UnlockSurface(dst);
        SDL_FreeSurface(dst);
        return NULL;
    }

    const float gauss_spatial[3][3] = {{0.075f, 0.124f, 0.075f},
                                       {0.124f, 0.204f, 0.124f},
                                       {0.075f, 0.124f, 0.075f}};
    const float sigma_intensity     = 20.0f;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int   idx = y * w + x;
            Uint8 r0, g0, b0, a0;
            SDL_GetRGBA(pixels[idx], src->format, &r0, &g0, &b0, &a0);

            float sum = 0.0f, wsum = 0.0f;
            for (int dy = -1; dy <= 1; dy++) {
                int yy = y + dy;
                if (yy < 0)
                    yy = 0;
                if (yy >= h)
                    yy = h - 1;
                for (int dx = -1; dx <= 1; dx++) {
                    int xx = x + dx;
                    if (xx < 0)
                        xx = 0;
                    if (xx >= w)
                        xx = w - 1;
                    Uint8 r1, g1, b1, a1;
                    SDL_GetRGBA(pixels[yy * w + xx], src->format, &r1, &g1, &b1,
                                &a1);
                    float diff = (float)(r1 - r0);
                    float w_intensity =
                        expf(-(diff * diff) /
                             (2 * sigma_intensity * sigma_intensity));
                    float w_total = gauss_spatial[dy + 1][dx + 1] * w_intensity;
                    sum += r1 * w_total;
                    wsum += w_total;
                }
            }
            Uint8 val       = (Uint8)(sum / wsum + 0.5f);
            tmp_pixels[idx] = SDL_MapRGBA(src->format, val, val, val, 255);
        }
    }

    uint32_t* integral = (uint32_t*)calloc(w * h, sizeof(uint32_t));
    if (!integral) {
        free(tmp_pixels);
        SDL_UnlockSurface(src);
        SDL_UnlockSurface(dst);
        SDL_FreeSurface(dst);
        return NULL;
    }

    for (int y = 0; y < h; y++) {
        uint32_t row_sum = 0;
        for (int x = 0; x < w; x++) {
            Uint8 r, g, b, a;
            SDL_GetRGBA(tmp_pixels[y * w + x], src->format, &r, &g, &b, &a);
            row_sum += r;
            if (y == 0)
                integral[y * w + x] = row_sum;
            else
                integral[y * w + x] = integral[(y - 1) * w + x] + row_sum;
        }
    }

    int block_size = 15;
    int C          = 10;
    int radius     = block_size / 2;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int x1 = x - radius;
            if (x1 < 0)
                x1 = 0;
            int y1 = y - radius;
            if (y1 < 0)
                y1 = 0;
            int x2 = x + radius;
            if (x2 >= w)
                x2 = w - 1;
            int y2 = y + radius;
            if (y2 >= h)
                y2 = h - 1;
            int count = (x2 - x1 + 1) * (y2 - y1 + 1);

            uint32_t sum = integral[y2 * w + x2];
            if (x1 > 0)
                sum -= integral[y2 * w + x1 - 1];
            if (y1 > 0)
                sum -= integral[(y1 - 1) * w + x2];
            if (x1 > 0 && y1 > 0)
                sum += integral[(y1 - 1) * w + x1 - 1];

            Uint8 r, g, b, a;
            SDL_GetRGBA(tmp_pixels[y * w + x], src->format, &r, &g, &b, &a);
            uint8_t thresh = (sum / count) - C;
            uint8_t val    = (r > thresh) ? 255 : 0;
            out_pixels[y * w + x] =
                SDL_MapRGBA(src->format, val, val, val, 255);
        }
    }

    free(tmp_pixels);
    free(integral);

    if (SDL_MUSTLOCK(src))
        SDL_UnlockSurface(src);
    if (SDL_MUSTLOCK(dst))
        SDL_UnlockSurface(dst);

    return dst;
}
