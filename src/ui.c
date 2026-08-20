
#include "ui.h"
#include "OCR.h"
#include "NN.h"
#include "solver.h"


// Fermeture propre de l'application
void on_window_destroy(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    if (widgets->current_filename) g_free(widgets->current_filename);
    g_free(widgets); // Libère la structure
    gtk_main_quit();
}

// Clic sur "Ouvrir Image"
void on_btn_open_clicked(GtkButton *btn, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;

    // Création de la boite de dialogue de choix de fichier
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Ouvrir une image",
                                                    widgets->window,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Annuler", GTK_RESPONSE_CANCEL,
                                                    "_Ouvrir", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Filtre pour ne voir que les images
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_mime_type(filter, "image/png");
    gtk_file_filter_add_mime_type(filter, "image/jpeg");
    gtk_file_filter_add_mime_type(filter, "image/bmp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Exécution de la boite de dialogue
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Sauvegarde du nom de fichier
        if (widgets->current_filename) g_free(widgets->current_filename);
        widgets->current_filename = g_strdup(filename);

        // Affichage dans l'interface (GtkImage)
        // Note: Pour de très grosses images, il faudrait redimensionner (GdkPixbuf)
        gtk_image_set_from_file(widgets->img_preview, filename);
        gtk_widget_set_opacity(GTK_WIDGET(widgets->img_preview), 1.0); // Rendre opaque (non grisé)

        // Réinitialiser la barre de progression et le texte
        gtk_progress_bar_set_fraction(widgets->progress_bar, 0.0);
        gtk_progress_bar_set_text(widgets->progress_bar, "Image chargée. Prêt.");
        
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(widgets->text_result);
        gtk_text_buffer_set_text(buffer, "", -1);

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Fonction appelée en boucle pour simuler le chargement (Animation)
gboolean simulation_step(gpointer user_data) {
    TimerData *data = (TimerData *)user_data;
    AppWidgets *widgets = data->widgets;

    data->progress += 0.05; // +5%

    if (data->progress >= 1.0) {
        // --- FIN DU TRAITEMENT ---
        gtk_progress_bar_set_fraction(widgets->progress_bar, 1.0);
        gtk_progress_bar_set_text(widgets->progress_bar, "Terminé !");
        
        // APPEL RÉEL À L'API ICI (Une fois la "simulation" finie ou directement)
        // char *result_text = api_ocr_run_inference(widgets->current_filename);
        
        // Afficher le résultat
        // GtkTextBuffer *buffer = gtk_text_view_get_buffer(widgets->text_result);
        // gtk_text_buffer_set_text(buffer, result_text, -1);
        
        // g_free(result_text);
        // g_free(data); // Libère la mémoire du timer
        
        // // Réactiver le bouton
        // gtk_widget_set_sensitive(GTK_WIDGET(widgets->btn_process), TRUE);
        
        return FALSE; // Arrête le timer
    }

    // Mise à jour barre
    gtk_progress_bar_set_fraction(widgets->progress_bar, data->progress);
    char buf[64];
    snprintf(buf, 64, "Analyse en cours... %.0f%%", data->progress * 100);
    gtk_progress_bar_set_text(widgets->progress_bar, buf);

    return TRUE; // Continue le timer
}

// Clic sur "Lancer Analyse"
void on_btn_process_clicked(GtkButton *btn, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;

    // Lancer l'animation (Simulation d'attente)
    TimerData *timer_data = g_malloc(sizeof(TimerData));
    timer_data->widgets = widgets;
    timer_data->progress = 0.0;


    if (widgets->current_filename == NULL) {
        gtk_progress_bar_set_text(widgets->progress_bar, "Erreur : Aucune image chargée !");
        return;
    }
    
    gtk_progress_bar_set_text(widgets->progress_bar, "Image Character Cutting...");


    int world_list_len = 0;
    int matrix_h_len = 0;
    int matrix_w_len = 0;

    
    // Préparer l'API (chargement matrice etc)
    if (ocr_load_image(widgets->current_filename, &world_list_len, &matrix_h_len, &matrix_w_len) == 0) {
        // gtk_progress_bar_set_text(widgets->progress_bar, "Erreur lors du chargement API.");
        // return;
    }else
    {
        gtk_progress_bar_set_text(widgets->progress_bar, "Image Characters Found!");
    }

    timer_data->progress = 30;

    printf("Here : %d, %d, %d\n", world_list_len, matrix_h_len, matrix_w_len);

    nn_images(widgets,world_list_len, matrix_h_len, matrix_w_len);
    
    timer_data->progress = 60;



    for(int i = 0; i < world_list_len; i++)
    {
        printf("Word : %s\n", widgets->word_list[i]);
    }

    for(int x = 0; x < matrix_h_len; x++)
    {
        for(int y = 0; y < matrix_w_len; y++)
        {
            printf("%c", widgets->matrix[x][y]);
        }
        printf("\n");
    }

    WordPos** res = solveCrosswords(widgets->matrix, matrix_h_len, matrix_w_len, widgets->word_list);
    
    // Désactiver le bouton pour éviter le double-clic
    gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);

    
    
    // Appelle 'simulation_step' toutes les 100ms
    g_timeout_add(100, simulation_step, timer_data);
}


void start_ui(int argc, char** argv, AppWidgets* widgets)
{
    gtk_init(&argc, &argv);

    // Allocation de la structure principale
    widgets->current_filename = NULL;

    GtkBuilder *builder = gtk_builder_new();
    GError *error = NULL;

    // 1. Chargement XML
    if (gtk_builder_add_from_file(builder, "data/interface.ui", &error) == 0) {
        g_printerr("Erreur chargement XML: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    // 2. Chargement CSS
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    if(gtk_css_provider_load_from_path(cssProvider, "data/style.css", &error) == 0){
        g_printerr("Erreur chargement CSS: %s\n", error->message);
        g_clear_error(&error);
    }
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    // 3. Récupération des widgets
    widgets->window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
    widgets->img_preview = GTK_IMAGE(gtk_builder_get_object(builder, "img_preview"));
    widgets->text_result = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "text_result"));
    widgets->progress_bar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "progress_bar"));
    widgets->btn_process = GTK_BUTTON(gtk_builder_get_object(builder, "btn_process"));

    // 4. Connexion manuelle des signaux (plus propre pour passer 'widgets' en argument)
    // Note: Dans le XML, on a mis handler="on_...", mais ici on connecte manuellement 
    // pour pouvoir passer la structure 'widgets' facilement. 
    // Si tu veux utiliser le XML automatique, il faut que 'widgets' soit global (pas recommandé).
    
    // On écrase les signaux du XML pour utiliser les notres avec data
    g_signal_connect(widgets->window, "destroy", G_CALLBACK(on_window_destroy), widgets);
    
    // Récupérer les boutons pour connecter
    GObject *btn_open = gtk_builder_get_object(builder, "btn_open");
    g_signal_connect(btn_open, "clicked", G_CALLBACK(on_btn_open_clicked), widgets);
    
    g_signal_connect(widgets->btn_process, "clicked", G_CALLBACK(on_btn_process_clicked), widgets);

    g_object_unref(builder);

    gtk_widget_show_all(GTK_WIDGET(widgets->window));
    gtk_main();
}