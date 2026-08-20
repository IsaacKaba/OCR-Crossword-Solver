#include "pretreatment.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN_SIZE 25
#define MAX_SIZE 675

SDL_Surface* denoise_image(const SDL_Surface* surf)
{
    if (!surf)
        return NULL;

    /* save of surfaec (writting in out) */
    SDL_Surface* out = SDL_ConvertSurface((SDL_Surface*)surf, surf->format, 0);
    if (!out)
        return NULL;

    int w = surf->w;
    int h = surf->h;

    /* allocation */
    Uint8* visited   = calloc(w * h, 1);
    Uint8* component = calloc(w * h, 1);
    if (!visited || !component) {
        free(visited);
        free(component);
        SDL_FreeSurface(out);
        return NULL;
    }

    typedef struct {
        int x, y;
    } Point;
    Point* stack = malloc(w * h * sizeof(Point));
    if (!stack) {
        free(visited);
        free(component);
        SDL_FreeSurface(out);
        return NULL;
    }

    /* lock surfaces before direct access to pixels */
    if (SDL_LockSurface((SDL_Surface*)surf) != 0) {
        fprintf(stderr, "SDL_LockSurface surf failed: %s\n", SDL_GetError());
        free(visited);
        free(component);
        free(stack);
        SDL_FreeSurface(out);
        return NULL;
    }
    if (SDL_LockSurface(out) != 0) {
        fprintf(stderr, "SDL_LockSurface out failed: %s\n", SDL_GetError());
        SDL_UnlockSurface((SDL_Surface*)surf);
        free(visited);
        free(component);
        free(stack);
        SDL_FreeSurface(out);
        return NULL;
    }

    Uint32* spixels = (Uint32*)surf->pixels;
    Uint32* opixels = (Uint32*)out->pixels;

    int colored_components = 0;

    for (int y0 = 0; y0 < h; y0++) {
        for (int x0 = 0; x0 < w; x0++) {
            int idx0 = y0 * w + x0;
            if (visited[idx0])
                continue;

            /* read color correctly  */
            Uint32 pix = spixels[idx0];
            Uint8  r, g, b, a;
            SDL_GetRGBA(pix, surf->format, &r, &g, &b, &a);

            /* here we suppose that image is binarised : black = (0,0,0), blank
             * = (255,255,255) */
            if (!(r == 0 && g == 0 && b == 0)) {
                visited[idx0] = 1; /* blank pixel, we mark it as visited */
                continue;
            }

            /* flood-fill for this component */
            int top         = 0;
            int count       = 0;
            stack[top++]    = (Point){x0, y0};
            visited[idx0]   = 1;
            component[idx0] = 1;

            while (top > 0) {
                Point p = stack[--top];
                count++;

                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = p.x + dx;
                        int ny = p.y + dy;
                        if (nx < 0 || ny < 0 || nx >= w || ny >= h)
                            continue;
                        int nidx = ny * w + nx;
                        if (visited[nidx])
                            continue;

                        Uint32 npix = spixels[nidx];
                        Uint8  nr, ng, nb, na;
                        SDL_GetRGBA(npix, surf->format, &nr, &ng, &nb, &na);
                        if (!(nr == 0 && ng == 0 && nb == 0)) {
                            visited[nidx] =
                                1; /* blank, marks as visited in component */
                            continue;
                        }
                        visited[nidx]   = 1;
                        component[nidx] = 1;
                        stack[top++]    = (Point){nx, ny};
                    }
                }
            }

            /* if the component is small then we color it in red (or we delete
             * it)
             */
            if (count < MIN_SIZE || count > MAX_SIZE) {
                colored_components++;
                for (int i = 0; i < w * h; i++) {
                    if (component[i]) {
                        opixels[i] =
                            SDL_MapRGBA(out->format, 255, 0, 0, 255); /* red */
                    }
                }
            }

            /* reset component mask for the next component */
            memset(component, 0, w * h);
        }
    }

    SDL_UnlockSurface((SDL_Surface*)surf);
    SDL_UnlockSurface(out);

    free(stack);
    free(visited);
    free(component);

    return out;
}
