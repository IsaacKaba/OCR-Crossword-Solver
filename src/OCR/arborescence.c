#include "image_splitter.h"

/**
 * Creates a single directory using `mkdir -p`.
 */
void creer_dossier_si_absent(const char* nom, char* debug_path)
{
    char commande[1024];

    // delete the folder and its content if it exist
    int len = snprintf(commande, sizeof(commande), "rm -rf \"%s\"", nom);
    if (len < 0 || len >= (int)sizeof(commande)) {
        fprintf(stderr, "Error: path too long (%s)\n", nom);
        return;
    }
    int res = system(commande);
    (void)res;

    // create empty folder
    len = snprintf(commande, sizeof(commande), "mkdir -p \"%s\"", nom);
    if (len < 0 || len >= (int)sizeof(commande)) {
        fprintf(stderr, "Error: path too long (%s)\n", nom);
        return;
    }
    res = system(commande);
    (void)res;
    if (debug_path)
        printf("Directory reset/created: %s\n", nom);
}

/**
 * Creates the complete directory structure for letter output.
 * Structure:
 * decoupage_lettres/
 * ├── lettres_grille/
 * └── lettres_liste_mots/
 *     ├── mot_0/
 *     ├── mot_1/
 *     └── ...
 */
void creer_arborescence(Grid* grid, const char* racine, char* debug_path)
{
    char chemin[1024];

    // Create the main root folder
    creer_dossier_si_absent(racine, debug_path);

    //  Create subfolders for grid letters and word list
    snprintf(chemin, sizeof(chemin), "%s/lettres_grille", racine);
    creer_dossier_si_absent(chemin, debug_path);

    snprintf(chemin, sizeof(chemin), "%s/lettres_liste_mots", racine);
    creer_dossier_si_absent(chemin, debug_path);

    // Create one subfolder for each word
    for (int i = 0; i < grid->word_count; i++) {
        snprintf(chemin, sizeof(chemin), "%s/lettres_liste_mots/mot_%d", racine,
                 i);
        creer_dossier_si_absent(chemin, debug_path);
    }
}
