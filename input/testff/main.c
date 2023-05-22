#include <gtk/gtk.h>

// Function declarations
//void on_file_choose_button_clicked(GtkButton *button, gpointer user_data);
//void on_use_live_video_button_clicked(GtkButton *button, gpointer user_data);
void on_window_destroy(GtkWidget *widget, gpointer user_data);
void on_file_choose_button_clicked(GtkButton *button, gpointer user_data);
void on_use_live_video_button_clicked(GtkButton *button, gpointer user_data);
void on_use_phone_video_button_clicked(GtkButton *button, gpointer user_data);


GtkBuilder *builder;
GtkWidget *file_chooser_button; // Add a variable to store the FileChooserButton widget
GtkTextBuffer *textBuffer; // Add a variable to store the TextBuffer of the TextView widget

int main(int argc, char *argv[]) {
    

    const char *glade_file_path = argv[1];

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, glade_file_path, NULL);

    // Get the main window with the ID 'main_window'
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));

    // Set the background color of the window
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-color: #a5dff9; }", -1, NULL);
    GtkStyleContext *style_context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Get the TextView widget with the ID 'textview_id'
    GtkWidget *textView = GTK_WIDGET(gtk_builder_get_object(builder, "textview_id"));
    textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));

    // Connect the "destroy" signal to the callback function
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Get the FileChooserButton with the ID 'file_chooser_button'
    file_chooser_button = GTK_WIDGET(gtk_builder_get_object(builder, "file_chooser_button"));

    // Get the use live video button with the ID 'use_live_video_button'
    GtkButton *button_use_live_video = GTK_BUTTON(gtk_builder_get_object(builder, "use_live_video_button"));
    g_signal_connect(button_use_live_video, "clicked", G_CALLBACK(on_use_live_video_button_clicked), NULL);

    GtkButton *button_use_phone_video = GTK_BUTTON(gtk_builder_get_object(builder, "use_live_phone_button"));
    g_signal_connect(button_use_phone_video, "clicked", G_CALLBACK(on_use_phone_video_button_clicked), NULL);

    GtkButton *button_convert = GTK_BUTTON(gtk_builder_get_object(builder, "convert"));
    g_signal_connect(button_convert, "clicked", G_CALLBACK(on_file_choose_button_clicked), NULL);

    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

void on_file_choose_button_clicked(GtkButton *button, gpointer user_data) {
    // Get the file path and immediately display the result in the text view
    gchar *file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser_button));

    // Check the file extension to determine whether it's an image or a video
    const gchar *extension = strrchr(file_path, '.');
    if (extension) {
        if (g_ascii_strcasecmp(extension, ".jpg") == 0  || g_ascii_strcasecmp(extension, ".bmp") == 0 || g_ascii_strcasecmp(extension, ".jpeg") == 0 || g_ascii_strcasecmp(extension, ".png") == 0) {
            // It's an image file

            // Perform image to ASCII conversion and obtain the ASCII result as a string
            char command[300];
            sprintf(command, "./Desktop/asciify/input/testff/testPipeFF %s %i %i", file_path, 3, 60); // Replace "ascii_converter_image" with the actual command for image conversion TODO
            system(command);

            // Update the content of the TextView widget with the ASCII result
            //GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
            //gtk_text_buffer_set_text(buffer, "ASCII result for image", -1);
        } else if (g_ascii_strcasecmp(extension, ".mp4") == 0 || g_ascii_strcasecmp(extension, ".avi") == 0 || g_ascii_strcasecmp(extension, ".mov") == 0) {
            // It's a video file

            // Perform video to ASCII conversion and obtain the ASCII frames as a string
            char command[300];
            sprintf(command, "./Desktop/asciify/input/testff/testPipeFF %s %i %i", file_path, 5, 30); // Replace "ascii_converter_video" with the actual command for video conversion TODO
            //on_window_destroy(button, user_data);
            system(command);

            // Update the content of the TextView widget with the ASCII frames
            //GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
            //gtk_text_buffer_set_text(buffer, "ASCII frames for video", -1);
        } else {
            // Unsupported file format
            g_print("Unsupported file format.\n");
        }
    }

    g_free(file_path);
}

void on_use_live_video_button_clicked(GtkButton *button, gpointer user_data) {
    // Do the live video conversion into ASCII characters
    // Replace "ascii_converter_live_video" with the actual command for live video conversion TODO
    system("./Desktop/asciify/input/testff/testPipeFF web_cam 5");

    // Update the content of the TextView widget with the live video ASCII frames
    //GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    //gtk_text_buffer_set_text(buffer, "ASCII frames for live video", -1);
}

void on_use_phone_video_button_clicked(GtkButton *button, gpointer user_data) {
    // Do the live video conversion into ASCII characters
    // Replace "ascii_converter_live_video" with the actual command for live video conversion TODO
    system("./Desktop/asciify/input/testff/testPipeFF phone_cam 5");

    // Update the content of the TextView widget with the live video ASCII frames
    //GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    //gtk_text_buffer_set_text(buffer, "ASCII frames for live video", -1);
}

void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    gtk_widget_destroy(widget);
    gtk_main_quit();
}
