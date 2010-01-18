#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>


#include "room.h"
int fd;
long total_distance = 0;
GtkWidget *room;


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


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int roombad_connect() {
	int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("roomba", "4000", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return -2;
    }

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

	if (fcntl(sockfd, F_SETFL, FNDELAY) < 0) {
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
		return -1 ;
	}	

	printf("client: connecting to %s\n", s);
	return sockfd;
}




static gboolean process_socket(gpointer data) {
	static GString *string = NULL;
	char buf[512];
	int res;
	
	if(string == NULL) {
		string = g_string_new(NULL);	
	}
	
	res = read(fd, buf, 512);
	if(res > 0) {
		string = g_string_append_len(string, buf, res);	
	}
	
	while(string->len >= 13) {
		if(string->str[0] == 'S' && string->str[1] == '|') {
			long s1, s2, s3, s4;
			struct _event e;
			sscanf(string->str+2, "%02d|%02d|%04d|%04d", &s1, &s2, &s3, &s4);
			total_distance += s3;
			egg_room_face_set_distance(room, total_distance);
			e.type = 'S';
			e.l1 = s3;
			egg_room_face_add_event(room, &e);

			g_string_erase(string, 0, 18);
		} else if(string->str[0] == 'W' && string->str[1] == '|') {
			long w1, w2;
			struct _event e;
			sscanf(string->str+2, "%05d|%05d", &w1, &w2);
			e.type = 'W';
			e.l1 = w1;
			e.l2 = w2;
			egg_room_face_add_event(room, &e);

			g_string_erase(string, 0, 13);
		} else {
			//All packets need to start with 19
			g_string_erase(string, 0, 1);	
		}
	}
	
	return TRUE;	
}

static gboolean move(gpointer data) {
	egg_room_face_redraw_canvas(room);
	return TRUE;
}

int main(int argc, char *argv[]) {
	GtkWidget *window;
	GtkWidget *button;
	
	fd = roombad_connect();
	if(fd < 0)
		exit(1);
	
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	button = gtk_button_new_with_label("Hello World");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(hello), NULL);
	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(window));


	room = egg_room_face_new ();
	gtk_container_add(GTK_CONTAINER (window), room);
	//gtk_widget_show(button);	
	gtk_widget_show_all(window);
	
	gtk_widget_set_usize(window, 800, 600);
	gtk_timeout_add(10, process_socket, NULL);
	gtk_timeout_add(100, move, NULL);
	gtk_main();
	return 0;
}

