#pragma once


#include <stdlib.h>
#include <err.h>
#include "neural_network.h"
#include "train.h"

void print_char(SDL_Surface *img);
char ocr(SDL_Surface *img);

int ocr_load_image(const char* image, int* world_list_len, int* matrix_h_len, int* matrix_w_len);