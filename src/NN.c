#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <limits.h>

#include "NN.h"

#define INPUT 784
#define HIDDEN 128
#define OUTPUT 26
#define SIZE INPUT
#define SEUIL 128
#define ALPHA 0.01
#define FILENAME "for_solver.txt"

// ----------------- UTILITAIRES -----------------
double rand_double(double min, double max) {
    double scale = rand() / (double)RAND_MAX;
    return min + scale * (max - min);
}

double rand_xavier(size_t n_in, size_t n_out) {
    double limit = sqrt(6.0) / sqrt(n_in + n_out);
    return rand_double(-limit, limit);
}

double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }
double dsigmoid(double x) { return x * (1.0 - x); }

void softmax(double *output, int num_outputs) {
    double max = output[0];
    for (int i = 1; i < num_outputs; i++) if (output[i] > max) max = output[i];
    double sum = 0;
    for (int i = 0; i < num_outputs; i++) { output[i] = exp(output[i] - max); sum += output[i]; }
    for (int i = 0; i < num_outputs; i++) output[i] /= sum;
}

// ----------------- NEURAL NET -----------------
neural_net *initialize() {
    neural_net *hey = malloc(sizeof(neural_net));
    if (!hey) errx(1,"malloc failed for neural_net");

    hey->LayerHidden = calloc(HIDDEN, sizeof(double));
    hey->LayerOut = calloc(OUTPUT, sizeof(double));
    hey->hidden_w = malloc(INPUT * sizeof(double*));
    hey->out_w = malloc(HIDDEN * sizeof(double*));
    hey->BiasHidden = malloc(HIDDEN * sizeof(double));
    hey->BiasOut = malloc(OUTPUT * sizeof(double));
    if (!hey->LayerHidden || !hey->LayerOut || !hey->hidden_w || !hey->out_w || !hey->BiasHidden || !hey->BiasOut)
        errx(1,"malloc failed");

    for (size_t i = 0; i < INPUT; i++) {
        hey->hidden_w[i] = malloc(HIDDEN * sizeof(double));
        if (!hey->hidden_w[i]) errx(1,"malloc failed");
    }
    for (size_t i = 0; i < HIDDEN; i++) {
        hey->out_w[i] = malloc(OUTPUT * sizeof(double));
        if (!hey->out_w[i]) errx(1,"malloc failed");
    }

    for (size_t i = 0; i < HIDDEN; i++) {
        for (size_t j = 0; j < INPUT; j++) hey->hidden_w[j][i] = rand_xavier(INPUT,HIDDEN);
        hey->BiasHidden[i] = rand_double(-0.1,0.1);
    }
    for (size_t i = 0; i < OUTPUT; i++) {
        for (size_t j = 0; j < HIDDEN; j++) hey->out_w[j][i] = rand_xavier(HIDDEN,OUTPUT);
        hey->BiasOut[i] = rand_double(-0.1,0.1);
    }

    return hey;
}

void free_neural_net(neural_net *hey) {
    if(!hey) return;
    for(size_t i=0;i<INPUT;i++) free(hey->hidden_w[i]);
    for(size_t i=0;i<HIDDEN;i++) free(hey->out_w[i]);
    free(hey->hidden_w);
    free(hey->out_w);
    free(hey->LayerHidden);
    free(hey->LayerOut);
    free(hey->BiasHidden);
    free(hey->BiasOut);
    free(hey);
}

// ----------------- PROPAGATION -----------------
void go_h(neural_net *hey, double *LayerIn) {
    for(size_t i=0;i<HIDDEN;i++){
        hey->LayerHidden[i]=0.0;
        for(size_t j=0;j<INPUT;j++) hey->LayerHidden[i]+=hey->hidden_w[j][i]*LayerIn[j];
        hey->LayerHidden[i]=sigmoid(hey->LayerHidden[i]+hey->BiasHidden[i]);
    }
}

void go_o(neural_net *hey){
    for(size_t i=0;i<OUTPUT;i++){
        hey->LayerOut[i]=0.0;
        for(size_t j=0;j<HIDDEN;j++) hey->LayerOut[i]+=hey->out_w[j][i]*hey->LayerHidden[j];
        hey->LayerOut[i]+=hey->BiasOut[i];
    }
    softmax(hey->LayerOut, OUTPUT);
}

// ----------------- PREDICTION -----------------
char predict(neural_net *hey, const char *image_path){
    double *in = here((char*)image_path);
    if(!in) return '?';
    go_h(hey,in);
    go_o(hey);
    free(in);

    size_t max=0;
    for(size_t i=0;i<OUTPUT;i++) if(hey->LayerOut[i]>hey->LayerOut[max]) max=i;

    if(hey->LayerOut[max]<0.5) return '?'; // confiance faible
    return (char)(max+'A');
}

// ----------------- INITIALIZE WITH MODEL -----------------
neural_net *initialize_with_model(const char *model_path){
    neural_net *hey = initialize();
    FILE *f=fopen(model_path,"r");
    if(!f){ free_neural_net(hey); errx(1,"Cannot open model file"); }

    for(size_t i=0;i<INPUT;i++)
        for(size_t j=0;j<HIDDEN;j++)
            if(fscanf(f,"%lf",&hey->hidden_w[i][j])!=1) errx(1,"Error reading hidden weights");
    for(size_t i=0;i<HIDDEN;i++)
        if(fscanf(f,"%lf",&hey->BiasHidden[i])!=1) errx(1,"Error reading hidden biases");
    for(size_t i=0;i<HIDDEN;i++)
        for(size_t j=0;j<OUTPUT;j++)
            if(fscanf(f,"%lf",&hey->out_w[i][j])!=1) errx(1,"Error reading output weights");
    for(size_t i=0;i<OUTPUT;i++)
        if(fscanf(f,"%lf",&hey->BiasOut[i])!=1) errx(1,"Error reading output biases");

    fclose(f);
    return hey;
}

// ----------------- IMAGE PROCESSING -----------------
double *convert(SDL_Surface *surface){
    if(!surface) return NULL;
    double *out=malloc(SIZE*sizeof(double));
    if(!out) return NULL;
    Uint8 *pixels=(Uint8*)surface->pixels;
    size_t i=0;
    for(int y=0;y<surface->h;y++){
        for(int x=0;x<surface->w;x++){
            Uint8 *p=pixels + y*surface->pitch + x*3;
            Uint8 r=p[0],g=p[1],b=p[2];
            Uint8 lum=(Uint8)(0.299*r + 0.587*g + 0.114*b);
            out[i++]=(lum<SEUIL)?0:1;
        }
    }
    return out;
}

double *here(const char *path) {
    SDL_Surface *t = IMG_Load(path);
    if (!t) errx(1, "Could not load image");
    if (t->w * t->h != SIZE) errx(1, "Image wrong size");
    SDL_Surface *surface = SDL_ConvertSurfaceFormat(t, SDL_PIXELFORMAT_RGB888, 0);
    SDL_LockSurface(surface);
    double *list = convert(surface);
    SDL_FreeSurface(t);
    SDL_FreeSurface(surface);
    return list;
}


// ----------------- Binarization avec filtre -----------------
SDL_Surface* bni(SDL_Surface* src){
    // … garde ton code bni complet ici …
    return src; // placeholder pour éviter erreurs compilation
}

// ----------------- WORD & MATRIX -----------------
int count_letters(const char *base_path){
    int n=0;
    while(1){
        char path[PATH_MAX];
        snprintf(path,sizeof(path),"%s/lettre_%d.png",base_path,n);
        FILE *f=fopen(path,"r");
        if(!f) break;
        fclose(f); n++;
    }
    return n;
}

char *build_word(neural_net *hey,const char *base_path){
    int len=count_letters(base_path);
    if(len<=0) return NULL;
    char *word=malloc(len+1);
    for(int i=0;i<len;i++){
        char path[PATH_MAX];
        snprintf(path,sizeof(path),"%s/lettre_%d.png",base_path,i);
        word[i]=predict(hey,path);
    }
    word[len]='\0';
    return word;
}

void build_matrix(AppWidgets *widgets,int word_list_len,int mat_h,int mat_w){
    widgets->word_list=malloc(sizeof(char*)*word_list_len);
    widgets->matrix=malloc(sizeof(char*)*mat_h);
    for(int i=0;i<mat_h;i++) widgets->matrix[i]=malloc(sizeof(char)*mat_w);
}

bool nn_images(AppWidgets *widgets,int word_list_len,int mat_h,int mat_w){
    neural_net *hey=initialize_with_model("data/hello.txt");
    build_matrix(widgets,word_list_len,mat_h,mat_w);
    const char *filepath="bin/ocr_assets";

    for(int i=0;i<word_list_len;i++){
        char path[PATH_MAX];
        snprintf(path,sizeof(path),"%s/lettres_liste_mots/mot_%d",filepath,i);
        widgets->word_list[i]=build_word(hey,path);
    }

    for(int x=0;x<mat_h;x++){
        for(int y=0;y<mat_w;y++){
            char path[PATH_MAX];
            snprintf(path,sizeof(path),"%s/lettres_grille/%d-%d.png",filepath,x,y);
            widgets->matrix[x][y]=predict(hey,path);
        }
    }

    free_neural_net(hey);
    return true;
}
