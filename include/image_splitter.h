#ifndef IMAGE_SPLITTER_H
#define IMAGE_SPLITTER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

// Structures
typedef struct {
    int x, y, w, h;
} Box;

typedef struct {
    Box* letters;
    int  letter_count;
} Word;

typedef struct {
    Box** chars;
    int   rows;
    int   cols;
    Word* words;
    int   word_count;
} Grid;

void save_letter_with_margin(SDL_Surface* image, Box b, const char* path,
                             int margin);
void decouper_lettres(SDL_Surface* image, Grid* grid, int margin,
                      char* debug_path);

void creer_dossier_si_absent(const char* nom, char* debug_path);
void creer_arborescence(Grid* grid, const char* racine, char* debug_path);

Box* extract_characters(SDL_Surface* surface, int* count);

int cmp_x(const void* a, const void* b);

void detect_grid_from_chars(Box* chars, int char_count, Box* grid,
                            Box* wordlist, int img_w, int img_h);
void ensure_grid_lines_sorted(Grid* grid);

Grid* build_grid_structure(Box* chars, int char_count, Box grid_box,
                           Box wordlist_box);

Grid* detect_grid_and_words(SDL_Surface* processed, int* char_count_out,
                            const char* debug_path);
void  decouper_grid_letters(SDL_Surface* image, Grid* grid,
                            const char* output_dir, int marge, char* debug_path);

void draw_rect(SDL_Surface* surf, SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b);
int  save_image(const char* path, SDL_Surface* surf);
void remove_short_words(Grid* grid, int n);
void remove_too_tall_letters(Grid* grid);
SDL_Surface* resize_surface_to_50x50(SDL_Surface* src);

void split_large_letters(Grid* grid);

#endif
