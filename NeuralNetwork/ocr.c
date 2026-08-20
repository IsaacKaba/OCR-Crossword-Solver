#include "ocr.h"

int main(int argc, char *argv[])
{
    if(argc > 3)
        errx(1, "usage: ./ocr <Path to file> or ./ocr -t <EPOCHS>");
    if(argc == 3)
    {
        if (strcmp(argv[1], "-t") != 0) {
            printf("Error: expected -t option\n");
            return 1;
        }

        char *endptr;
        size_t epochs = strtoul(argv[2], &endptr, 10);
        ocr_train(epochs, 25, 100);
    }
    else
    {
        SDL_Surface *img;
        img = load_image(argv[1]);
        binarize(img);
        print_char(img);
        printf("Character = %c\n", ocr(img));
    }
    return 0;
}

void print_char(SDL_Surface *img)
{
    Uint32 pixel;
    Uint8 r, g , b;
    printf("##############################\n");
    for(int i = 0; i < img->w; i++)
    {
        printf("#");
        for(int j = 0; j < img->h; j++)
        {
            pixel = get_pixel(img,j,i);
            SDL_GetRGB(pixel, img->format, &r, &g, &b);
            printf("%s", r > 127 ? "#" : " ");
        }
        printf("#\n");
    }
    printf("##############################\n");
}

void ocr_train(size_t n, int print, int save)
{
    struct network net = load_network("tmp/ocr.nn");
    struct train t = load_ocr_train("../dataset/");
    for(size_t i = 0; i <= n; i++)
    {
        train_network(net, t, (i % print) == 0, "OCR");
        if((i % print) == 0)
            printf("EPOCH = %lu\n", i);
        if((i % save) == 0)
            save_network(net, "tmp/ocr.nn");
    }
    free_train(t);
    free_network(net);
}

char ocr(SDL_Surface *img)
{
    struct network net = load_network("tmp/ocr.nn");
    double inputs[784];
    make_in(img, inputs);
    compute_network(net, inputs);
    free_network(net);
    return what_is_out(output_network(net), 62);
}
