#include "pretreatment.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Uint8* binarize_adaptive(const SDL_Surface* surf)
{
    int    w   = surf->w;
    int    h   = surf->h;
    Uint8* bin = malloc(w * h);
    if (!bin)
        return NULL;

    if (SDL_MUSTLOCK(surf))
        SDL_LockSurface((SDL_Surface*)surf);
    Uint32* pixels = (Uint32*)surf->pixels;

    double mean = 0;
    for (int i = 0; i < w * h; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], surf->format, &r, &g, &b, &a);
        mean += r;
    }
    mean /= (w * h);

    for (int i = 0; i < w * h; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], surf->format, &r, &g, &b, &a);
        bin[i] = (r < mean) ? 1 : 0;
    }

    if (SDL_MUSTLOCK(surf))
        SDL_UnlockSurface((SDL_Surface*)surf);
    return bin;
}

double projection_variance(Uint8* bin, int w, int h, double angle)
{
    double rad   = angle * M_PI / 180.0;
    double cos_a = cos(rad);
    double sin_a = sin(rad);

    int ncx = w / 2;
    int ncy = h / 2;

    double* hist = calloc(h, sizeof(double));
    if (!hist)
        return 0;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            double tx = x - ncx;
            double ty = y - ncy;
            int    sx = (int)(+tx * cos_a + ty * sin_a + ncx);
            int    sy = (int)(-tx * sin_a + ty * cos_a + ncy);
            if (sx >= 0 && sx < w && sy >= 0 && sy < h) {
                hist[y] += bin[sy * w + sx];
            }
        }
    }

    double mean = 0;
    for (int i = 0; i < h; i++)
        mean += hist[i];
    mean /= h;

    double var = 0;
    for (int i = 0; i < h; i++)
        var += (hist[i] - mean) * (hist[i] - mean);
    var /= h;

    free(hist);
    return var;
}

double estimate_skew_angle(const SDL_Surface* surf, double max_angle,
                           double step)
{
    Uint8* bin = binarize_adaptive(surf);
    if (!bin)
        return 0;

    double best_angle = 0;
    double best_var   = 0;

    for (double angle = -max_angle; angle <= max_angle; angle += step) {
        double var = projection_variance(bin, surf->w, surf->h, angle);
        if (var > best_var) {
            best_var   = var;
            best_angle = angle;
        }
    }

    free(bin);
    return best_angle;
}

SDL_Surface* deskew_image(const SDL_Surface* surf, char* debug_path)
{
    double angle = estimate_skew_angle(surf, 45.0, 0.2);
    if (debug_path)
        printf("Detected angle: %.2f degrees\n", angle);
    fflush(stdout);
    return rotate_image(surf, angle);
}
