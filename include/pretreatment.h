#ifndef PRETREATMENT_H
#define PRETREATMENT_H
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>

/* =========================================================
 *  IMAGE PREPROCESSING MODULE (draft)
 *  ---------------------------------------------------------
 *  Provides image manipulation tools for OCR preprocessing.
 *  Includes functions for loading, saving, cleaning, rotating,
 *  contrast adjustment, binarization, and grid/character detection.
 * ========================================================= */

/* ---------- Basic Image Utilities ---------- */

/**
 * Load an image from the given file path into an SDL_Surface.
 * Returns NULL if loading fails.
 */
SDL_Surface* load_image(const char* path);

/**
 * Save an SDL_Surface as a PNG file at the specified path.
 * Returns 0 on success, -1 on error.
 */
int save_image(const char* path, SDL_Surface* surf);

/**
 * Check if a pixel is black according to the given format.
 * Returns 1 if black, 0 otherwise.
 */
int is_black(Uint32 pixel, SDL_PixelFormat* format);

/**
 * Convert the image to grayscale.
 * Modifies the surface in-place.
 */
void convert_to_grayscale(SDL_Surface* surf);

/* ---------- Image Transformations ---------- */

/**
 * Rotate an image by a specified angle in degrees.
 * Returns a new rotated surface (must be freed later).
 */
SDL_Surface* rotate_image(const SDL_Surface* src, double angle);

/**
 * Estimate the skew angle (rotation) of the image.
 * Scans through the range [-max_angle, max_angle] with the given step.
 * Returns the best estimated skew angle in degrees.
 */
double estimate_skew_angle(const SDL_Surface* surf, double max_angle,
                           double step);

/**
 * Automatically correct image skew using the estimated angle.
 * Returns a new deskewed surface.
 */
SDL_Surface* deskew_image(const SDL_Surface* surf, char* debug_path);

/* ---------- Image Filtering ---------- */

/**
 * Apply a 3x3 median filter to reduce noise.
 * Returns a new denoised surface.
 */
SDL_Surface* denoise_image(const SDL_Surface* surf);

/**
 * Enhance image contrast using linear stretching.
 * Returns a new contrast-enhanced surface.
 */
SDL_Surface* enhance_contrast(const SDL_Surface* surf);

/**
 * Binarize an image (convert grayscale to black & white).
 * Returns a new binarized surface.
 */
SDL_Surface* binarize_image(SDL_Surface* src);

SDL_Surface* preprocess_image(const char* input_file, const char* debug_path,
                              double rotate_angle, int rotate_set);

#endif // PRETREATMENT_H
