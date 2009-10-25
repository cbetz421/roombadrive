#include <gtk/gtk.h>
#include "room.h"

static void hello(GtkWidget *widget, gpointer data) {
	g_print("Hello World\n");
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
	gtk_main_quit();
	return TRUE;
}

static void destroy(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

static gboolean process_socket(gpointer data) {


	return TRUE;	
}

int main(int argc, char *argv[]) {
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *clock;


	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	button = gtk_button_new_with_label("Hello World");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(hello), NULL);
	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));


	clock = egg_room_face_new ();
	gtk_container_add(GTK_CONTAINER (window), clock);
	//gtk_widget_show(button);	
	gtk_widget_show_all(window);
	
	gtk_widget_set_usize(window, 800, 600);
	gtk_timeout_add(10, process_socket, NULL);
	gtk_main();
	return 0;
}

