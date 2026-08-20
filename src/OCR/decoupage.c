#include "image_splitter.h"

/**
 * save_letter_with_margin
 * -----------------------
 * Extracts a letter from the main image (the crossword grid or word list)
 * and saves it as an individual BMP image with a white margin around it.
 *
 */
void save_letter_with_margin(SDL_Surface* image, Box b, const char* path,
                             int margin)
{
    int new_w = b.w + 2 * margin;
    int new_h = b.h + 2 * margin;

    //  new SDL surface : letter + margin
    SDL_Surface* lettre = SDL_CreateRGBSurface(
        0, new_w, new_h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    // new surface with white
    SDL_FillRect(lettre, NULL, SDL_MapRGB(lettre->format, 255, 255, 255));

    // Define the source rectangle
    SDL_Rect src = {b.x, b.y, b.w, b.h};

    // Define the destination position
    SDL_Rect dst = {margin, margin, b.w, b.h};

    // Copy  the letter pixels
    SDL_BlitSurface(image, &src, lettre, &dst);

    SDL_Surface* final_img = resize_surface_to_50x50(lettre);

    SDL_SaveBMP(final_img, path);

    SDL_FreeSurface(lettre);
    SDL_FreeSurface(final_img);
}

/**
 * decouper_lettres
 * ----------------
 * Iterates through all the letter positions in the crossword grid and word
 * list, extracts each letter image, and saves it as a separate file using
 * save_letter_with_margin().
 *
 */
void decouper_lettres(SDL_Surface* image, Grid* grid, int margin,
                      char* debug_path)
{
    //   grid
    for (int i = 0; i < grid->rows; i++) {
        for (int j = 0; j < grid->cols; j++) {
            char path[128];
            // Create the output file path
            sprintf(path,
                    "bin/ocr_assets/lettres_grille/%d-%d.png",
                    i, j);

            // Extract and save the letter from the main image
            save_letter_with_margin(image, grid->chars[i][j], path, margin);

            // debug
            if (debug_path)
                printf("→ Grid letter [%d][%d] saved: %s\n", i, j, path);
        }
    }

    //  word list
    for (int i = 0; i < grid->word_count; i++) {
        for (int j = 0; j < grid->words[i].letter_count; j++) {
            char path[128];
            // Create the output file path
            sprintf(path,
                    "bin/ocr_assets/lettres_liste_mots/mot_%d/"
                    "lettre_%d.png",
                    i, j);

            // Extract and save the letter from the main image
            save_letter_with_margin(image, grid->words[i].letters[j], path,
                                    margin);

            // debug
            if (debug_path)
                printf("→ Word letter [%d][%d] saved: %s\n", i, j, path);
        }
    }
}
