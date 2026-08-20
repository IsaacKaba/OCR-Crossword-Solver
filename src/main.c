
#include "ui.h"


int main(int argc, char** argv)
{

    AppWidgets *widgets = g_malloc(sizeof(AppWidgets));
    //* Start UI
    start_ui(argc, argv, widgets);


    return 0;
}