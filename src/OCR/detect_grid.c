#include "image_splitter.h"

int cmp_x(const void* a, const void* b)
{
    Box* A = (Box*)a;
    Box* B = (Box*)b;
    return A->x - B->x;
}
int cmp_yx(const void* a, const void* b)
{
    Box* ba = (Box*)a;
    Box* bb = (Box*)b;
    if (ba->y != bb->y)
        return ba->y - bb->y;
    return ba->x - bb->x;
}

#define MAX_TAILLE 350

Word* build_words(Box* chars, int char_count, Box wordlist_box,
                  int* word_count_out)
{
    if (!chars || char_count <= 0 || word_count_out == NULL)
        return NULL;

    Box* letters = malloc(sizeof(Box) * char_count);
    int  count   = 0;
    for (int i = 0; i < char_count; i++) {
        Box b = chars[i];
        if (b.x >= wordlist_box.x &&
            b.x + b.w <= wordlist_box.x + wordlist_box.w &&
            b.y >= wordlist_box.y &&
            b.y + b.h <= wordlist_box.y + wordlist_box.h) {
            letters[count++] = b;
        }
    }

    if (count == 0) {
        free(letters);
        *word_count_out = 0;
        return NULL;
    }

    qsort(letters, count, sizeof(Box), cmp_yx);

    Word* words = malloc(sizeof(Word) * count);
    int   widx  = 0;

    int line_y0 = letters[0].y;
    int line_y1 = letters[0].y + letters[0].h;

    Box* line_letters = malloc(sizeof(Box) * count);
    int  line_count   = 0;

    for (int i = 0; i < count; i++) {
        Box b = letters[i];
        if (b.y > line_y1 + 2) {
            if (line_count > 0) {
                qsort(line_letters, line_count, sizeof(Box), cmp_x);

                double total_space = 0.0;
                for (int j = 1; j < line_count; j++)
                    total_space += line_letters[j].x - (line_letters[j - 1].x +
                                                        line_letters[j - 1].w);
                double avg_space =
                    (line_count > 1) ? total_space / (line_count - 1) : 0.0;
                double threshold = avg_space * 3;

                int m_start = 0;
                for (int j = 1; j < line_count; j++) {
                    double gap = line_letters[j].x - (line_letters[j - 1].x +
                                                      line_letters[j - 1].w);
                    if (gap > threshold) {
                        int word_start = m_start;
                        int word_end   = j - 1;
                        int word_width = (line_letters[word_end].x +
                                          line_letters[word_end].w) -
                                         line_letters[word_start].x;

                        if (word_width <= MAX_TAILLE) {
                            int letters_in_word = word_end - word_start + 1;
                            words[widx].letters =
                                malloc(sizeof(Box) * letters_in_word);
                            words[widx].letter_count = letters_in_word;
                            for (int k = 0; k < letters_in_word; k++)
                                words[widx].letters[k] =
                                    line_letters[word_start + k];
                            widx++;
                        }
                        m_start = j;
                    }
                }

                int word_start = m_start;
                int word_end   = line_count - 1;
                int word_width =
                    (line_letters[word_end].x + line_letters[word_end].w) -
                    line_letters[word_start].x;
                if (word_width <= MAX_TAILLE) {
                    int letters_in_word = word_end - word_start + 1;
                    words[widx].letters = malloc(sizeof(Box) * letters_in_word);
                    words[widx].letter_count = letters_in_word;
                    for (int k = 0; k < letters_in_word; k++)
                        words[widx].letters[k] = line_letters[word_start + k];
                    widx++;
                }
            }

            line_letters[0] = b;
            line_count      = 1;
            line_y0         = b.y;
            line_y1         = b.y + b.h;
        } else {
            if (b.y < line_y0)
                line_y0 = b.y;
            if (b.y + b.h > line_y1)
                line_y1 = b.y + b.h;
            line_letters[line_count++] = b;
        }
    }

    if (line_count > 0) {
        qsort(line_letters, line_count, sizeof(Box), cmp_x);

        double total_space = 0.0;
        for (int j = 1; j < line_count; j++)
            total_space += line_letters[j].x -
                           (line_letters[j - 1].x + line_letters[j - 1].w);
        double avg_space =
            (line_count > 1) ? total_space / (line_count - 1) : 0.0;
        double threshold = avg_space * 1.5;

        int m_start = 0;
        for (int j = 1; j < line_count; j++) {
            double gap = line_letters[j].x -
                         (line_letters[j - 1].x + line_letters[j - 1].w);
            if (gap > threshold) {
                int word_start = m_start;
                int word_end   = j - 1;
                int word_width =
                    (line_letters[word_end].x + line_letters[word_end].w) -
                    line_letters[word_start].x;

                if (word_width <= MAX_TAILLE) {
                    int letters_in_word = word_end - word_start + 1;
                    words[widx].letters = malloc(sizeof(Box) * letters_in_word);
                    words[widx].letter_count = letters_in_word;
                    for (int k = 0; k < letters_in_word; k++)
                        words[widx].letters[k] = line_letters[word_start + k];
                    widx++;
                }
                m_start = j;
            }
        }

        int word_start = m_start;
        int word_end   = line_count - 1;
        int word_width = (line_letters[word_end].x + line_letters[word_end].w) -
                         line_letters[word_start].x;
        if (word_width <= MAX_TAILLE) {
            int letters_in_word      = word_end - word_start + 1;
            words[widx].letters      = malloc(sizeof(Box) * letters_in_word);
            words[widx].letter_count = letters_in_word;
            for (int k = 0; k < letters_in_word; k++)
                words[widx].letters[k] = line_letters[word_start + k];
            widx++;
        }
    }

    free(line_letters);
    free(letters);

    *word_count_out = widx;
    return words;
}

void filter_boxes_by_height(Box* boxes, int* count)
{
    if (*count <= 0)
        return;

    float total_h = 0.0f;
    for (int i = 0; i < *count; i++)
        total_h += boxes[i].h;
    float avg_h = total_h / *count;

    for (int i = 0; i < *count; i++) {
        if (boxes[i].h < avg_h * 0.7f || boxes[i].h > avg_h * 1.3f) {
            boxes[i] = boxes[*count - 1];
            (*count)--;
            i--;
        }
    }
}

Grid* build_grid_structure(Box* chars, int char_count, Box grid_box,
                           Box wordlist_box)
{
    if (!chars || char_count <= 0)
        return NULL;

    Box* grid_letters = malloc(sizeof(Box) * char_count);
    int  gcount       = 0;
    for (int i = 0; i < char_count; i++) {
        Box b = chars[i];
        if (b.x >= grid_box.x && b.x + b.w <= grid_box.x + grid_box.w &&
            b.y >= grid_box.y && b.y + b.h <= grid_box.y + grid_box.h) {
            grid_letters[gcount++] = b;
        }
    }
    if (gcount == 0) {
        free(grid_letters);
        return NULL;
    }

    qsort(grid_letters, gcount, sizeof(Box), cmp_yx);

    int  max_rows      = gcount;
    int* row_starts    = malloc(sizeof(int) * max_rows);
    int  rows          = 0;
    int  line_y0       = grid_letters[0].y;
    int  line_y1       = grid_letters[0].y + grid_letters[0].h;
    row_starts[rows++] = 0;

    for (int i = 1; i < gcount; i++) {
        Box b = grid_letters[i];
        if (b.y > line_y1 + 2) {
            row_starts[rows++] = i;
            line_y0            = b.y;
            line_y1            = b.y + b.h;
        } else {
            if (b.y + b.h > line_y1)
                line_y1 = b.y + b.h;
        }
    }

    int cols = 0;
    for (int r = 0; r < rows; r++) {
        int start   = row_starts[r];
        int end     = (r < rows - 1) ? row_starts[r + 1] : gcount;
        int row_len = end - start;
        if (row_len > cols)
            cols = row_len;
    }

    Box** grid_chars = malloc(sizeof(Box*) * rows);
    for (int r = 0; r < rows; r++) {
        int start   = row_starts[r];
        int end     = (r < rows - 1) ? row_starts[r + 1] : gcount;
        int row_len = end - start;

        grid_chars[r] = malloc(sizeof(Box) * cols);
        for (int c = 0; c < cols; c++) {
            if (c < row_len)
                grid_chars[r][c] = grid_letters[start + c];
            else
                grid_chars[r][c] = (Box){0, 0, 0, 0};
        }
    }

    free(grid_letters);
    free(row_starts);

    int   word_count = 0;
    Word* words = build_words(chars, char_count, wordlist_box, &word_count);

    Grid* grid       = malloc(sizeof(Grid));
    grid->rows       = rows;
    grid->cols       = cols;
    grid->chars      = grid_chars;
    grid->words      = words;
    grid->word_count = word_count;

    return grid;
}

void ensure_grid_lines_sorted(Grid* grid)
{
    if (!grid || !grid->chars)
        return;

    for (int r = 0; r < grid->rows; r++) {
        Box* line   = grid->chars[r];
        bool sorted = true;

        // check if the lign is already sorted
        for (int c = 1; c < grid->cols; c++) {
            if (line[c - 1].x > line[c].x) {
                sorted = false;
                break;
            }
        }

        // if not sorted then bubble sort
        if (!sorted) {
            for (int i = 0; i < grid->cols - 1; i++) {
                for (int j = 0; j < grid->cols - i - 1; j++) {
                    if (line[j].x > line[j + 1].x) {
                        Box tmp     = line[j];
                        line[j]     = line[j + 1];
                        line[j + 1] = tmp;
                    }
                }
            }
        }
    }
}

static int box_distance(const Box* a, const Box* b)
{
    int ax2 = a->x + a->w - 1;
    int ay2 = a->y + a->h - 1;
    int bx2 = b->x + b->w - 1;
    int by2 = b->y + b->h - 1;

    int dx = 0;
    if (ax2 < b->x)
        dx = b->x - ax2;
    else if (bx2 < a->x)
        dx = a->x - bx2;
    else
        dx = 0;

    int dy = 0;
    if (ay2 < b->y)
        dy = b->y - ay2;
    else if (by2 < a->y)
        dy = a->y - by2;
    else
        dy = 0;

    return (dx > dy) ? dx : dy;
}

static int uf_find(int* parent, int x)
{
    if (parent[x] != x)
        parent[x] = uf_find(parent, parent[x]);
    return parent[x];
}

static void uf_union(int* parent, int* rank, int a, int b)
{
    int ra = uf_find(parent, a);
    int rb = uf_find(parent, b);
    if (ra == rb)
        return;
    if (rank[ra] < rank[rb])
        parent[ra] = rb;
    else if (rank[rb] < rank[ra])
        parent[rb] = ra;
    else {
        parent[rb] = ra;
        rank[ra]++;
    }
}

void detect_grid_from_chars(Box* chars, int char_count, Box* grid,
                            Box* wordlist, int img_w, int img_h)
{
    if (!chars || char_count <= 0 || !grid || !wordlist)
        return;

    grid->x = grid->y = grid->w = grid->h = 0;
    wordlist->x = wordlist->y = wordlist->w = wordlist->h = 0;

    double avg_w = 0.0, avg_h = 0.0;
    for (int i = 0; i < char_count; i++) {
        avg_w += chars[i].w;
        avg_h += chars[i].h;
    }
    avg_w /= char_count;
    avg_h /= char_count;
    double avg_size = (avg_w + avg_h) * 0.5;
    if (avg_size < 1.0)
        avg_size = 1.0;
    int dist_thresh = (int)ceil(avg_size * 1.9);
    if (dist_thresh < 3)
        dist_thresh = 3;

    int* parent = (int*)malloc(sizeof(int) * char_count);
    int* rank   = (int*)calloc(char_count, sizeof(int));
    for (int i = 0; i < char_count; i++)
        parent[i] = i;

    for (int i = 0; i < char_count; i++) {
        for (int j = i + 1; j < char_count; j++) {
            if (box_distance(&chars[i], &chars[j]) <= dist_thresh) {
                uf_union(parent, rank, i, j);
            }
        }
    }

    int* cluster_id    = (int*)malloc(sizeof(int) * char_count);
    int* root_to_index = (int*)malloc(sizeof(int) * char_count);
    for (int i = 0; i < char_count; i++) {
        cluster_id[i]    = -1;
        root_to_index[i] = -1;
    }

    Box* cluster_bbox  = (Box*)malloc(sizeof(Box) * char_count);
    int* cluster_count = (int*)malloc(sizeof(int) * char_count);
    int  clusters      = 0;

    for (int i = 0; i < char_count; i++) {
        int r = uf_find(parent, i);
        if (root_to_index[r] == -1) {
            root_to_index[r]        = clusters;
            cluster_bbox[clusters]  = chars[i];
            cluster_count[clusters] = 1;
            cluster_id[i]           = clusters;
            clusters++;
        } else {
            int  idx = root_to_index[r];
            Box* cb  = &cluster_bbox[idx];
            int  x0  = (cb->x < chars[i].x) ? cb->x : chars[i].x;
            int  y0  = (cb->y < chars[i].y) ? cb->y : chars[i].y;
            int  x1  = ((cb->x + cb->w) > (chars[i].x + chars[i].w))
                           ? (cb->x + cb->w)
                           : (chars[i].x + chars[i].w);
            int  y1  = ((cb->y + cb->h) > (chars[i].y + chars[i].h))
                           ? (cb->y + cb->h)
                           : (chars[i].y + chars[i].h);
            cb->x    = x0;
            cb->y    = y0;
            cb->w    = x1 - x0;
            cb->h    = y1 - y0;
            cluster_count[idx]++;
            cluster_id[i] = idx;
        }
    }

    int    best_idx   = -1;
    int    best_count = 0;
    double best_score = -1.0;
    for (int i = 0; i < clusters; i++) {
        int    cnt    = cluster_count[i];
        Box*   cb     = &cluster_bbox[i];
        double aspect = (cb->h > 0) ? ((double)cb->w / (double)cb->h) : 1.0;
        if (aspect < 1.0)
            aspect = 1.0 / aspect;
        double score = (double)cnt - (aspect - 1.0) * 0.3 * (double)cnt;
        if (best_idx == -1 || score > best_score) {
            best_idx   = i;
            best_score = score;
            best_count = cnt;
        }
    }

    if (best_idx != -1 && best_count >= 4) {
        *grid   = cluster_bbox[best_idx];
        int pad = (int)ceil(avg_size * 0.5);
        if (pad < 1)
            pad = 1;
        grid->x = (grid->x - pad >= 0) ? grid->x - pad : 0;
        grid->y = (grid->y - pad >= 0) ? grid->y - pad : 0;
        grid->w += 2 * pad;
        grid->h += 2 * pad;
        if (grid->x + grid->w > img_w)
            grid->w = img_w - grid->x;
        if (grid->y + grid->h > img_h)
            grid->h = img_h - grid->y;

    } else
        grid->x = grid->y = grid->w = grid->h = 0;

    int any_word = 0, wx0 = INT_MAX, wy0 = INT_MAX, wx1 = 0, wy1 = 0;
    for (int i = 0; i < char_count; i++) {
        if (cluster_id[i] == best_idx)
            continue;
        Box* b = &chars[i];
        if (b->w <= 0 || b->h <= 0)
            continue;

        int gx0 = grid->x, gy0 = grid->y, gx1 = grid->x + grid->w,
            gy1 = grid->y + grid->h;
        int bx0 = b->x, by0 = b->y, bx1 = b->x + b->w, by1 = b->y + b->h;
        int inter_w = (bx1 > gx0 ? (bx0 < gx1 ? (bx1 < gx1 ? bx1 : gx1) -
                                                    (bx0 > gx0 ? bx0 : gx0)
                                              : 0)
                                 : 0);
        int inter_h = (by1 > gy0 ? (by0 < gy1 ? (by1 < gy1 ? by1 : gy1) -
                                                    (by0 > gy0 ? by0 : gy0)
                                              : 0)
                                 : 0);

        if (inter_w > 0 && inter_h > 0) {
            continue;
        }

        any_word = 1;
        if (b->x < wx0)
            wx0 = b->x;
        if (b->y < wy0)
            wy0 = b->y;
        if (b->x + b->w > wx1)
            wx1 = b->x + b->w;
        if (b->y + b->h > wy1)
            wy1 = b->y + b->h;
    }

    if (any_word) {
        wordlist->x = wx0;
        wordlist->y = wy0;
        wordlist->w = wx1 - wx0;
        wordlist->h = wy1 - wy0;
        int padw    = (int)ceil(avg_w * 0.5);
        int padh    = (int)ceil(avg_h * 0.5);
        if (padw < 1)
            padw = 1;
        if (padh < 1)
            padh = 1;
        wordlist->x = (wordlist->x - padw >= 0) ? wordlist->x - padw : 0;
        wordlist->y = (wordlist->y - padh >= 0) ? wordlist->y - padh : 0;
        wordlist->w += 2 * padw;
        wordlist->h += 2 * padh;
        if (wordlist->x + wordlist->w > img_w)
            wordlist->w = img_w - wordlist->x;
        if (wordlist->y + wordlist->h > img_h)
            wordlist->h = img_h - wordlist->y;

    } else {
        wordlist->x = wordlist->y = wordlist->w = wordlist->h = 0;
    }

    free(parent);
    free(rank);
    free(cluster_id);
    free(root_to_index);
    free(cluster_bbox);
    free(cluster_count);
}

static double word_center_x(const Word* w)
{
    if (!w || !w->letters || w->letter_count == 0)
        return 0.0;
    double sum = 0.0;
    for (int i = 0; i < w->letter_count; i++)
        sum += (w->letters[i].x + w->letters[i].w / 2.0);
    return sum / w->letter_count;
}

static double word_center_y(const Word* w)
{
    if (!w || !w->letters || w->letter_count == 0)
        return 0.0;
    double sum = 0.0;
    for (int i = 0; i < w->letter_count; i++)
        sum += (w->letters[i].y + w->letters[i].h / 2.0);
    return sum / w->letter_count;
}

static double word_distance(const Word* a, const Word* b)
{
    double dx = word_center_x(a) - word_center_x(b);
    double dy = word_center_y(a) - word_center_y(b);
    return sqrt(dx * dx + dy * dy);
}

static Word merge_words(const Word* a, const Word* b)
{
    Word merged;
    merged.letter_count = a->letter_count + b->letter_count;
    merged.letters      = malloc(sizeof(Box) * merged.letter_count);
    if (!merged.letters) {
        merged.letter_count = 0;
        return merged;
    }

    for (int i = 0; i < a->letter_count; i++)
        merged.letters[i] = a->letters[i];
    for (int i = 0; i < b->letter_count; i++)
        merged.letters[a->letter_count + i] = b->letters[i];
    return merged;
}

void remove_short_words(Grid* grid, int n)
{
    if (!grid || !grid->words || grid->word_count <= 0)
        return;

    int  count        = grid->word_count;
    int* merged_flags = calloc(count, sizeof(int));
    if (!merged_flags)
        return;

    Word* merged_list = malloc(sizeof(Word) * count);
    if (!merged_list) {
        free(merged_flags);
        return;
    }
    int merged_count = 0;

    // merge small words wth near one's
    for (int i = 0; i < count; i++) {
        if (merged_flags[i])
            continue;
        Word* w = &grid->words[i];
        if (!w->letters)
            continue;

        if (w->letter_count <= n) {
            int    best_j    = -1;
            double best_dist = 999999.0;
            for (int j = 0; j < count; j++) {
                if (i == j || merged_flags[j])
                    continue;
                double dist = word_distance(w, &grid->words[j]);
                if (dist < best_dist && dist < 50.0) {
                    best_dist = dist;
                    best_j    = j;
                }
            }

            if (best_j != -1) {
                Word new_w = merge_words(w, &grid->words[best_j]);
                merged_list[merged_count++] = new_w;
                merged_flags[i] = merged_flags[best_j] = 1;
                continue;
            }
        }

        // else keep the word unchange
        if (!merged_flags[i]) {
            merged_list[merged_count++] = *w;
            merged_flags[i]             = 1;
        }
    }

    free(merged_flags);
    free(grid->words);
    grid->words      = merged_list;
    grid->word_count = merged_count;

    // Delete word still too small after merging
    Word* temp_list = malloc(sizeof(Word) * merged_count);
    if (!temp_list)
        return;
    int temp_count = 0;

    for (int i = 0; i < merged_count; i++) {
        if (grid->words[i].letter_count > n) {
            temp_list[temp_count++] = grid->words[i];
        } else {
            free(grid->words[i].letters);
        }
    }

    free(grid->words);
    grid->words      = temp_list;
    grid->word_count = temp_count;

    if (temp_count <= 1)
        return;

    // Delete isolate words
    double sum_y = 0.0;
    for (int i = 0; i < temp_count; i++)
        sum_y += word_center_y(&grid->words[i]);
    double avg_y = sum_y / temp_count;

    double sum_dev = 0.0;
    for (int i = 0; i < temp_count; i++)
        sum_dev += fabs(word_center_y(&grid->words[i]) - avg_y);
    double avg_dev = sum_dev / temp_count;

    double threshold = avg_dev * 4.0;

    Word* final_list = malloc(sizeof(Word) * temp_count);
    if (!final_list)
        return;
    int final_count = 0;

    for (int i = 0; i < temp_count; i++) {
        double cy = word_center_y(&grid->words[i]);
        if (fabs(cy - avg_y) <= threshold) {
            final_list[final_count++] = grid->words[i];
        } else {
            free(grid->words[i].letters); // delete isolate words
        }
    }

    free(grid->words);
    grid->words      = final_list;
    grid->word_count = final_count;
}

void remove_too_tall_letters(Grid* grid)
{
    if (!grid || !grid->words || grid->word_count <= 0)
        return;

    for (int i = 0; i < grid->word_count; i++) {
        Word* w = &grid->words[i];
        if (!w->letters || w->letter_count <= 0)
            continue;

        double sum_h = 0.0;
        for (int j = 0; j < w->letter_count; j++) {
            sum_h += w->letters[j].h;
        }
        double avg_h = sum_h / w->letter_count;

        Box* filtered = malloc(sizeof(Box) * w->letter_count);
        if (!filtered)
            continue;

        int kept = 0;
        for (int j = 0; j < w->letter_count; j++) {
            if (w->letters[j].h <= avg_h * 1.24) {
                filtered[kept++] = w->letters[j];
            }
        }

        free(w->letters);
        w->letters      = filtered;
        w->letter_count = kept;
    }
}

void split_large_letters(Grid* grid)
{
    if (!grid || !grid->words)
        return;

    const float tolerance = 1.65f;

    for (int w = 0; w < grid->word_count; w++) {
        Word* word = &grid->words[w];
        if (word->letter_count <= 0)
            continue;

        float avg = 0.0f;
        for (int i = 0; i < word->letter_count; i++)
            avg += word->letters[i].w;
        avg /= word->letter_count;

        int last = word->letter_count - 1;
        Box b    = word->letters[last];

        if (b.w <= avg * tolerance)
            continue;

        int half  = b.w / 2;
        Box left  = (Box){b.x, b.y, half, b.h};
        Box right = (Box){b.x + half, b.y, b.w - half, b.h};

        int  new_count   = word->letter_count + 1;
        Box* new_letters = malloc(sizeof(Box) * new_count);

        for (int i = 0; i < last; i++)
            new_letters[i] = word->letters[i];

        new_letters[last]     = left;
        new_letters[last + 1] = right;

        free(word->letters);
        word->letters      = new_letters;
        word->letter_count = new_count;
    }
}
