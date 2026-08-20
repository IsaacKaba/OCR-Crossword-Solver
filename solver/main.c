#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "solver.h"


void print_mat(char** mat, int rows)
{
    for(int i = 0; i < rows; i++)
    {
        printf("%s\n", mat[i]);
    }
}


void free_mat(char** mat, int cols, int rows)
{
    for(int i = 0; i < rows; i++)
    {
        free(mat[i]);
    }
    free(mat);
}

void print_pos(WordPos* pos)
{
    printf("(%u, %u)(%u, %u)\n", pos->x1, pos->y1, pos->x2, pos->y2);
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        errx(1, "[Error] : args must be GRID - WORD");
    }


    const char* grid_path = argv[1];
    const char* word = argv[2];

    if(strlen(word) <= 0)
    {
        errx(1, "[Error] : word cant be empty");
    }



    FILE* file = fopen(grid_path, "r");
    if(!file){
        errx(1, "[Error] : grid file not found");
    }

    char* line = NULL;
    size_t len = 0;
    size_t lines_count = 0;
    ssize_t n;
    
    char** mat = NULL;
    int cols = 0;
    int rows = 0;

    while((n = getline(&line, &len, file)) != -1)
    {
        int c_count = 0;
        for(ssize_t i = 0; i < n; i++)
        {
            if(line[i] != '\n')
                c_count++;
        }

        if (cols == 0) {
            cols = c_count;
        } else if (c_count != cols) {
            errx(1, "[Error] : all lines must have the same length");
        }


        char* tmp = malloc(sizeof(char) * (c_count + 1));
        
        strncpy(tmp, line, c_count);
        tmp[c_count] = '\0';

        rows += 1;
        mat = realloc(mat, rows * sizeof(char*));
        mat[rows - 1] = tmp; 

        lines_count++;
    }


    print_mat(mat, rows);
    

    WordPos res = {0}; 
    res = findWord(mat, cols, rows, word);
    print_pos(&res);

    free(line);
    free_mat(mat, cols, rows);

    return 0;
}