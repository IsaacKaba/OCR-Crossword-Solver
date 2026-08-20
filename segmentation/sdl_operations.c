#include "sdl_operations.h"
#include <err.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

/* Globals for SDL2 window/renderer so we can render surfaces */
static SDL_Window   *gWindow   = NULL;
static SDL_Renderer *gRenderer = NULL;

/* Initialize SDL (video) and SDL_image if needed */
void init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(1, "Could not initialize SDL: %s.\n", SDL_GetError());

    /* Initialize SDL_image for common formats (PNG/JPG) */
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags)
        errx(1, "Could not initialize SDL_image: %s.\n", IMG_GetError());
}

SDL_Surface* load_image(char *path)
{
    SDL_Surface *img;

    img = IMG_Load(path);
    if (!img)
        errx(3, "can't load %s: %s", path, IMG_GetError());

    return img;
}

/* Create window/renderer when first displaying an image, render it, return original surface */
SDL_Surface* display_image(SDL_Surface *img)
{
    if (!img)
        errx(1, "display_image: img is NULL");

    /* Create window + renderer on first call (size = image size) */
    if (gWindow == NULL) {
        gWindow = SDL_CreateWindow("Image",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   img->w, img->h,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
        if (!gWindow)
            errx(1, "Couldn't create window: %s", SDL_GetError());

        gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!gRenderer) {
            SDL_DestroyWindow(gWindow);
            gWindow = NULL;
            errx(1, "Couldn't create renderer: %s", SDL_GetError());
        }
    } else {
        /* If window exists, optionally resize it to match image */
        SDL_SetWindowSize(gWindow, img->w, img->h);
    }

    /* Create a texture from the surface and render it */
    SDL_Texture *tex = SDL_CreateTextureFromSurface(gRenderer, img);
    if (!tex)
        warnx("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
    else {
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, tex, NULL, NULL);
        SDL_RenderPresent(gRenderer);
        SDL_DestroyTexture(tex);
    }

    return img; /* keep existing API: return the passed surface */
}

/* Wait for key press and release (uses SDL event API) */
void wait_for_keypressed()
{
    SDL_Event event;

    /* Wait for keydown */
    for (;;) {
        if (SDL_WaitEvent(&event) == 0) continue;
        if (event.type == SDL_KEYDOWN) break;
        if (event.type == SDL_QUIT) return;
    }

    /* Wait for keyup */
    for (;;) {
        if (SDL_WaitEvent(&event) == 0) continue;
        if (event.type == SDL_KEYUP) break;
        if (event.type == SDL_QUIT) return;
    }
}

/* Pixel helpers (same approach as before) */
static inline Uint8* pixel_ref(SDL_Surface *surf, unsigned x, unsigned y)
{
    int bpp = surf->format->BytesPerPixel;
    return (Uint8*)surf->pixels + y * surf->pitch + x * bpp;
}

Uint32 get_pixel(SDL_Surface *surface, unsigned x, unsigned y)
{
    Uint8 *p = pixel_ref(surface, x, y);

    switch (surface->format->BytesPerPixel)
    {
        case 1:
            return *p;

        case 2:
            return *(Uint16 *)p;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4:
            return *(Uint32 *)p;
    }

    return 0;
}

void put_pixel(SDL_Surface *surface, unsigned x, unsigned y, Uint32 pixel)
{
    Uint8 *p = pixel_ref(surface, x, y);

    switch(surface->format->BytesPerPixel)
    {
        case 1:
            *p = (Uint8)pixel;
            break;

        case 2:
            *(Uint16 *)p = (Uint16)pixel;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
}

/* Update the displayed surface (ignores 'screen' surface, uses renderer) */
void update_surface(SDL_Surface* screen, SDL_Surface* image)
{
    (void)screen; /* keep parameter to preserve API (unused) */

    if (!gRenderer) {
        warnx("update_surface: renderer not initialized");
        return;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(gRenderer, image);
    if (!tex) {
        warnx("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        return;
    }

    SDL_RenderClear(gRenderer);
    SDL_RenderCopy(gRenderer, tex, NULL, NULL);
    SDL_RenderPresent(gRenderer);
    SDL_DestroyTexture(tex);
}

/* Make image square by padding with white background */
SDL_Surface* square_char(SDL_Surface* image){
    if(image->w == image->h)
        return image;
    else{
        Uint32 pixel;
        pixel = SDL_MapRGB(image->format, 255, 255, 255);
        SDL_Surface* temp = NULL;
        if(image->w > image->h){
            temp = SDL_CreateRGBSurfaceWithFormat(0, image->w, image->w, 32, image->format->format);
            if (!temp) errx(1, "square_char: failed to create surface: %s", SDL_GetError());

            if (SDL_MUSTLOCK(temp)) SDL_LockSurface(temp);
            for(int i = 0; i < temp->w; i++){
                for(int j = 0; j < temp->w; j++)
                    put_pixel(temp, i ,j, pixel);
            }
            if (SDL_MUSTLOCK(temp)) SDL_UnlockSurface(temp);

            int average = (image->w - image->h) / 2;

            if (SDL_MUSTLOCK(temp)) SDL_LockSurface(temp);
            if (SDL_MUSTLOCK(image)) SDL_LockSurface(image);
            for(int i = 0; i < image->w; i++){
                for(int j = 0; j < image->h; j++)
                    put_pixel(temp, i, j + average, get_pixel(image, i, j));
            }
            if (SDL_MUSTLOCK(image)) SDL_UnlockSurface(image);
            if (SDL_MUSTLOCK(temp)) SDL_UnlockSurface(temp);
        }
        else{
            temp = SDL_CreateRGBSurfaceWithFormat(0, image->h, image->h, 32, image->format->format);
            if (!temp) errx(1, "square_char: failed to create surface: %s", SDL_GetError());

            if (SDL_MUSTLOCK(temp)) SDL_LockSurface(temp);
            for(int i = 0; i < temp->h; i++){
                for(int j = 0; j < temp->h; j++)
                    put_pixel(temp, i, j, pixel);
            }
            if (SDL_MUSTLOCK(temp)) SDL_UnlockSurface(temp);

            int average = (image->h - image->w) / 2;

            if (SDL_MUSTLOCK(temp)) SDL_LockSurface(temp);
            if (SDL_MUSTLOCK(image)) SDL_LockSurface(image);
            for(int i = 0; i < image->w; i++){
                for(int j = 0; j < image->h; j++)
                    put_pixel(temp, i + average, j, get_pixel(image, i, j));
            }
            if (SDL_MUSTLOCK(image)) SDL_UnlockSurface(image);
            if (SDL_MUSTLOCK(temp)) SDL_UnlockSurface(temp);
        }
        return temp;
    }
}

/* Resize wrapper that uses Resize() helper then postprocesses */
SDL_Surface* resize_char(SDL_Surface* image, int width, int height){
    SDL_Surface *temp = NULL;
    temp = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, image->format->format);

    if(temp == NULL)
        errx(1, "Failed to create the resize image: %s", SDL_GetError());

    if(image->w < 20)
        image = square_char(image);

    image = Resize(image, width, height);
    grayscale(image);
    binarize_char(image);
    return image;
}

/* More precise binarization for characters */
void binarize_char(SDL_Surface *img){
    Uint32 pxl;
    Uint8 r, g, b;
    int W = img->w, H = img->h;

    if (SDL_MUSTLOCK(img)) SDL_LockSurface(img);
    for(int i = 0; i < W; i++){
        for(int j = 0; j < H; j++){
            pxl = get_pixel(img, i, j);
            SDL_GetRGB(pxl, img->format, &r, &g, &b);
            r = r >= 10 ? 255 : 0;
            g = g >= 10 ? 255 : 0;
            b = b >= 10 ? 255 : 0;
            pxl = SDL_MapRGB(img->format, r, g, b);
            put_pixel(img, i, j, pxl);
        }
    }
    if (SDL_MUSTLOCK(img)) SDL_UnlockSurface(img);
}

void binarize(SDL_Surface *img)
{
    printf("Binar\n");
    Uint32 pixel;
    Uint8 r, g , b;
    int  w = img -> w, h = img -> h;

    if (SDL_MUSTLOCK(img)) SDL_LockSurface(img);
    for(int i = 0; i < w; i++){
        for(int j = 0; j < h; j++){
            pixel = get_pixel(img,i,j);
            SDL_GetRGB(pixel, img->format, &r, &g, &b);
            r = r >= 127 ? 255 : 0;
            g = g >= 127 ? 255 : 0;
            b = b >= 127 ? 255 : 0;
            pixel = SDL_MapRGB(img->format, r, g, b);
            put_pixel(img, i, j, pixel);
        }
    }
    if (SDL_MUSTLOCK(img)) SDL_UnlockSurface(img);
}

void grayscale(SDL_Surface *img)
{
    int width = img->w;
    int height = img->h;

    if (SDL_MUSTLOCK(img)) SDL_LockSurface(img);
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            Uint32 pixel = get_pixel(img, x, y);
            Uint8 r, g, b;
            SDL_GetRGB(pixel, img->format, &r, &g, &b);
            int average = (int)(0.3 * r + 0.59 * g + 0.11 * b);
            r = average; g = average; b = average;
            Uint32 pixel1 = SDL_MapRGB(img->format, r, g, b);
            put_pixel(img, x, y, pixel1);
        }
    }
    if (SDL_MUSTLOCK(img)) SDL_UnlockSurface(img);
}

/* Resize helper using SDL_BlitScaled (replacement for SDL_SoftStretch) */
SDL_Surface* Resize(SDL_Surface *img, int width, int height)
{
    /* Create destination surface with same pixel format as source */
    SDL_Surface *dest = SDL_CreateRGBSurfaceWithFormat(0,
                                  width,
                                  height,
                                  img->format->BitsPerPixel,
                                  img->format->format);
    if (!dest)
        errx(1, "Resize: failed to create dest surface: %s", SDL_GetError());

    /* Use SDL_BlitScaled to scale image */
    if (SDL_BlitScaled(img, NULL, dest, NULL) != 0) {
        SDL_FreeSurface(dest);
        errx(1, "Resize: SDL_BlitScaled failed: %s", SDL_GetError());
    }

    return dest;
}
