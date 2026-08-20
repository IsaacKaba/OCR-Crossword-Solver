#include "pretreatment.h"
#include "image_splitter.h"

int ocr_load_image(const char* image, int* world_list_len, int* matrix_h_len, int* matrix_w_len)
{
    // Initialisation SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    char* debug_path = NULL;

    // ===========================================
    // PHASE 1 : PRÉTRAITEMENT
    // ===========================================
    
    printf("L'image charge Monsieur Bouchet...\n");
    SDL_Surface* original = load_image(image);
    if (!original) {
        SDL_Quit();
        return 1;
    }

    printf("Conversion en niveaux de gris...\n");
    convert_to_grayscale(original);

    printf("Correction de l'inclinaison...\n");
    SDL_Surface* deskewed = deskew_image(original, debug_path);
    SDL_FreeSurface(original);

    printf("Amélioration du contraste...\n");
    SDL_Surface* contrasted = enhance_contrast(deskewed);
    SDL_FreeSurface(deskewed);

    printf("Binarisation...\n");
    SDL_Surface* binary = binarize_image(contrasted);
    SDL_FreeSurface(contrasted);

    printf("Débruitage Finale...\n");
    SDL_Surface* clean = denoise_image(binary);
    SDL_FreeSurface(binary);

    if (debug_path) {
        save_image("debug_preprocessed.png", clean);
        printf("→ Image prétraitée sauvegardée : debug_preprocessed.png\n");
    }

    // ===========================================
    // PHASE 2 : EXTRACTION DES CARACTÈRES
    // ===========================================
    
    printf("\nExtraction des caractères...\n");
    int char_count;
    Box* characters = extract_characters(clean, &char_count);
    printf("→ %d caractères détectés\n", char_count);

    if (char_count == 0) {
        fprintf(stderr, "erreur Aucun caractère détecté !\n");
        SDL_FreeSurface(clean);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // ===========================================
    // PHASE 3 : DÉTECTION DE LA STRUCTURE
    // ===========================================
    
    printf("\n📊 Détection de la grille et de la liste de mots...\n");
    Box grid_box, wordlist_box;
    detect_grid_from_chars(characters, char_count, 
                          &grid_box, &wordlist_box,
                          clean->w, clean->h);

    printf("→ Grille : [%d, %d, %d×%d]\n", 
           grid_box.x, grid_box.y, grid_box.w, grid_box.h);
    printf("→ Liste de mots : [%d, %d, %d×%d]\n",
           wordlist_box.x, wordlist_box.y, wordlist_box.w, wordlist_box.h);

    // ===========================================
    // PHASE 4 : CONSTRUCTION DE LA GRILLE
    // ===========================================
    
    printf("\nConstruction de la structure...\n");
    Grid* grid = build_grid_structure(characters, char_count,
                                      grid_box, wordlist_box);
    
    if (!grid) {
        fprintf(stderr, "Échec de construction de la grille !\n");
        free(characters);
        SDL_FreeSurface(clean);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    printf("→ Grille : %d lignes × %d colonnes\n", grid->rows, grid->cols);
    printf("→ Liste : %d mots détectés\n", grid->word_count);

    // Trier les lignes de la grille
    ensure_grid_lines_sorted(grid);

    // Nettoyages
    printf("\nNettoyage de la structure...\n");
    remove_short_words(grid, 2);        // Garde mots > 2 lettres
    remove_too_tall_letters(grid);      // Supprime lettres anormales
    split_large_letters(grid);          // Divise lettres collées

    printf("→ %d mots après nettoyage\n", grid->word_count);

    // ===========================================
    // PHASE 5 : DÉCOUPAGE ET EXPORT
    // ===========================================
    
    printf("\n✂️  Découpage des lettres...\n");
    creer_arborescence(grid, "bin/ocr_assets/", debug_path);
    decouper_lettres(clean, grid, 5, debug_path); // Marge de 5 pixels

    printf("\nPipeline terminé avec succès !\n");
    printf("Images disponibles dans : assets/decoupage_lettres/\n");
    printf("\nStatistiques finales :\n");
    printf("   - Grille : %d × %d = %d lettres\n", 
           grid->rows, grid->cols, grid->rows * grid->cols);
    printf("   - Mots : %d\n", grid->word_count);
    int total_word_letters = 0;
    for (int i = 0; i < grid->word_count; i++) {
        total_word_letters += grid->words[i].letter_count;
    }
    printf("   - Lettres dans les mots : %d\n", total_word_letters);


    *world_list_len = grid->word_count;
    *matrix_h_len = grid->rows;
    *matrix_w_len = grid->cols; 

    // ===========================================
    // LIBÉRATION DE LA MÉMOIRE
    // ===========================================
    
    for (int i = 0; i < grid->rows; i++)
        free(grid->chars[i]);
    free(grid->chars);
    
    for (int i = 0; i < grid->word_count; i++)
        free(grid->words[i].letters);
    free(grid->words);
    
    free(grid);
    free(characters);
    SDL_FreeSurface(clean);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
