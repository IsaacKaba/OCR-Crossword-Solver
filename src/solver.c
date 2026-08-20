#include "solver.h"

char to_lower(char c)
{
	if(c >= 'A' && c <= 'Z')
	{
		return c + 32;
	}
	return c;
}

WordPos findWord(char** matrix, unsigned int cols, unsigned int rows, const char* word)
{
    WordPos pos = {0, 0, 0, 0};
    size_t word_len = strlen(word);
    if(word_len <= 0) errx(1, "[Word List Error]: Words must have a size");

    int dirX[8] = {0, 0, 1, -1, 1, 1, -1, -1};
    int dirY[8] = {1, -1, 0, 0, 1, -1, 1, -1};

    for(unsigned int i = 0; i < rows; i++)      // lignes
    {
        for(unsigned int j = 0; j < cols; j++)  // colonnes
        {
            if(to_lower(matrix[i][j]) == to_lower(word[0]))
            {
                for(int d = 0; d < 8; d++)
                {
                    int x = j, y = i;
                    size_t k;
                    for(k = 1; k < word_len; k++)
                    {
                        x += dirX[d];
                        y += dirY[d];

                        if(x < 0 || x >= (int)cols || y < 0 || y >= (int)rows)
                            break;

                        if(to_lower(matrix[y][x]) != to_lower(word[k]))
                            break;
                    }

                    if(k == word_len)
                    {
                        pos.x1 = j;
                        pos.y1 = i;
                        pos.x2 = x;
                        pos.y2 = y;
                        return pos;
                    }
                }
            }
        }
    }

    return pos;
}



WordPos** solveCrosswords(char** matrix, unsigned int cols, unsigned int rows, const char** words)
{
	//* words** must be a null terminated array (or add size)
	
	//* alloc new WordPos array or create a huge array of words

	//* Size for 50 (to change)
	WordPos** finded_words_pos = malloc(50 * sizeof(WordPos*));
	if(!finded_words_pos) errx(1, "[Solver Error]: Error while allocating Word Pos Array");

	size_t i = 0;
	while(words[i])
	{
		WordPos pos = findWord(matrix, cols, rows, words[i]);
		WordPos* p = (WordPos*)malloc(sizeof(WordPos));
		if(!p)
		{
			errx(1, "[Solver Error]: Error while allocating memory for Word Pos");
		}

		*p = pos;
	}

	return finded_words_pos;
}

