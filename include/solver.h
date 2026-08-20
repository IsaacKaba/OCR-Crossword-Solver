#pragma once

#include <string.h>
#include <err.h>
#include <stdlib.h>

typedef struct {
	unsigned int x1, x2, y1, y2;
} WordPos;


WordPos findWord(char** matrix, unsigned int cols, unsigned int rows, const char* word);
WordPos** solveCrosswords(char** matrix, unsigned int cols, unsigned int rows, const char** words);
