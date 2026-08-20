#pragma once

#include <gtk/gtk.h>
#include <stdlib.h> // Pour free()

typedef struct {
    GtkWindow *window;
    GtkImage *img_preview;
    GtkTextView *text_result;
    GtkProgressBar *progress_bar;
    GtkButton *btn_process;
    gchar *current_filename; // Chemin du fichier image chargé
    char** matrix;
    char** word_list;
} AppWidgets;

// Petite structure pour passer des données au timeout (animation)
typedef struct {
    AppWidgets *widgets;
    double progress;
} TimerData;


void start_ui(int argc, char** argv, AppWidgets* widgets);
void on_btn_process_clicked(GtkButton *btn, gpointer data);
gboolean simulation_step(gpointer user_data);
void on_btn_open_clicked(GtkButton *btn, gpointer data);
void on_window_destroy(GtkWidget *widget, gpointer data);