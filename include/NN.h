#pragma once

#include "ui.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

// ----------------- Structures -----------------
typedef struct neural_net {
    double *LayerHidden;
    double *LayerOut;
    double **hidden_w;
    double **out_w;
    double *BiasHidden;
    double *BiasOut;
} neural_net;

double *here(const char *path);
neural_net *initialize_with_model(const char *model_path);

void build_matrix(AppWidgets* widgets, int word_list_len, int mat_h, int mat_w);
bool nn_images(AppWidgets* widgets, int word_list_len, int mat_h, int mat_w);

char predict(neural_net *hey, const char *image_path);