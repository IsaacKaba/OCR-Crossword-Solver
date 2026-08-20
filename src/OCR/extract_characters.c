#include "image_splitter.h"

#define MARGIN 0
#define MIN_SIZE_H 12
#define MIN_SIZE_W 3
#define MAX_SIZE 75
#define MERGE_DIST 0

int is_black(Uint32 pixel, SDL_PixelFormat* format)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel, format, &r, &g, &b);
    int brightness = (r + g + b) / 3;
    return brightness < 128;
}

void merge_boxes(Box* boxes, int* count)
{
    int merged;
    do {
        merged = 0;
        for (int i = 0; i < *count; i++) {
            for (int j = i + 1; j < *count; j++) {
                Box a = boxes[i];
                Box b = boxes[j];

                int close_x = (b.x <= a.x + a.w + MERGE_DIST) &&
                              (b.x + b.w + MERGE_DIST >= a.x);
                int close_y = (b.y <= a.y + a.h + MERGE_DIST) &&
                              (b.y + b.h + MERGE_DIST >= a.y);

                if (close_x && close_y) {
                    int min_x = (a.x < b.x) ? a.x : b.x;
                    int min_y = (a.y < b.y) ? a.y : b.y;
                    int max_x =
                        ((a.x + a.w) > (b.x + b.w)) ? (a.x + a.w) : (b.x + b.w);
                    int max_y =
                        ((a.y + a.h) > (b.y + b.h)) ? (a.y + a.h) : (b.y + b.h);

                    boxes[i].x = min_x;
                    boxes[i].y = min_y;
                    boxes[i].w = max_x - min_x;
                    boxes[i].h = max_y - min_y;

                    boxes[j] = boxes[*count - 1];
                    (*count)--;
                    merged = 1;
                    break;
                }
            }
            if (merged)
                break;
        }
    } while (merged);
}

Box* extract_characters(SDL_Surface* surface, int* count)
{
    int     width  = surface->w;
    int     height = surface->h;
    Uint32* pixels = (Uint32*)surface->pixels;

    int  max_chars = 2048;
    Box* boxes     = malloc(max_chars * sizeof(Box));
    *count         = 0;

    int** visited = malloc(height * sizeof(int*));
    for (int y = 0; y < height; y++)
        visited[y] = calloc(width, sizeof(int));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (!visited[y][x] &&
                is_black(pixels[y * width + x], surface->format)) {
                int min_x = x, max_x = x;
                int min_y = y, max_y = y;

                int  stack_size = width * height;
                int* stack_x    = malloc(stack_size * sizeof(int));
                int* stack_y    = malloc(stack_size * sizeof(int));
                int  sp         = 0;

                stack_x[sp] = x;
                stack_y[sp] = y;
                sp++;

                while (sp > 0) {
                    sp--;
                    int cx = stack_x[sp];
                    int cy = stack_y[sp];

                    if (cx < 0 || cx >= width || cy < 0 || cy >= height)
                        continue;
                    if (visited[cy][cx])
                        continue;
                    if (!is_black(pixels[cy * width + cx], surface->format))
                        continue;

                    visited[cy][cx] = 1;

                    if (cx < min_x)
                        min_x = cx;
                    if (cx > max_x)
                        max_x = cx;
                    if (cy < min_y)
                        min_y = cy;
                    if (cy > max_y)
                        max_y = cy;

                    stack_x[sp] = cx + 1;
                    stack_y[sp] = cy;
                    sp++;
                    stack_x[sp] = cx - 1;
                    stack_y[sp] = cy;
                    sp++;
                    stack_x[sp] = cx;
                    stack_y[sp] = cy + 1;
                    sp++;
                    stack_x[sp] = cx;
                    stack_y[sp] = cy - 1;
                    sp++;
                }

                free(stack_x);
                free(stack_y);

                int bx = (min_x - MARGIN < 0) ? 0 : min_x - MARGIN;
                int by = (min_y - MARGIN < 0) ? 0 : min_y - MARGIN;
                int bw = (max_x - min_x + 1) + 2 * MARGIN;
                int bh = (max_y - min_y + 1) + 2 * MARGIN;

                if (bx + bw > width)
                    bw = width - bx;
                if (by + bh > height)
                    bh = height - by;

                if (bw >= MIN_SIZE_W && bh >= MIN_SIZE_H && bh <= MAX_SIZE) {
                    if (*count < max_chars) {
                        boxes[*count].x = bx;
                        boxes[*count].y = by;
                        boxes[*count].w = bw;
                        boxes[*count].h = bh;
                        (*count)++;
                    }
                }
            }
        }
    }

    for (int y = 0; y < height; y++)
        free(visited[y]);
    free(visited);

    for (int i = 0; i < *count; i++) {
        Box b            = boxes[i];
        int white_count  = 0;
        int total_pixels = b.w * b.h;

        for (int y = b.y; y < b.y + b.h; y++) {
            for (int x = b.x; x < b.x + b.w; x++) {
                Uint32 pixel = pixels[y * width + x];
                if (!is_black(pixel, surface->format))
                    white_count++;
            }
        }
        if ((float)white_count / total_pixels >= 0.80f) {
            boxes[i] = boxes[*count - 1];
            (*count)--;
            i--;
        }
    }

    float total_w = 0;
    for (int i = 0; i < *count; i++)
        total_w += boxes[i].w;
    float avg_w     = (float)total_w / *count;
    float tolerance = avg_w >= 15 ? 1.768f : 2.0f;

    int original_count = *count;
    for (int i = 0; i < original_count; i++) {
        if (boxes[i].w >= avg_w * tolerance) {
            int k = (int)((boxes[i].w / avg_w) + 0.5f);
            if (k > 1) {
                int split_w = boxes[i].w / k;

                for (int s = 0; s < k; s++) {
                    if (*count < max_chars) {
                        boxes[*count].x = boxes[i].x + s * split_w;
                        boxes[*count].y = boxes[i].y;
                        boxes[*count].w = (s == k - 1)
                                              ? (boxes[i].x + boxes[i].w -
                                                 (boxes[i].x + s * split_w))
                                              : split_w;
                        boxes[*count].h = boxes[i].h;
                        (*count)++;
                    }
                }

                boxes[i] = boxes[*count - 1];
                (*count)--;
                i--;
                original_count--;
            }
        }
    }

    float total_h = 0.0f;
    for (int i = 0; i < *count; i++) {
        total_h += boxes[i].h;
    }
    float avg_h = (*count > 0) ? total_h / *count : 0.0f;
    for (int i = 0; i < *count; i++) {
        if (boxes[i].h < avg_h * 0.4f || boxes[i].h > avg_h * 1.6f) {
            boxes[i] = boxes[*count - 1];
            (*count)--;
            i--;
        }
    }

    return boxes;
}
