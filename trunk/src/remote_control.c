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

int sock;

int left_speed=10;
int right_speed=10;

void go_forward() {
	char buf[128];
	short l,r;

	g_print("going forward at speeds of %d:%d\n",left_speed,right_speed);

	// TODO: use htons
	l=left_speed;
	r=right_speed;
	buf[0] = 'W';
	memcpy(buf+1,&l,2);
	memcpy(buf+3,&r,2);

//	sprintf(buf,"W",speed);
	send(sock, buf, 5, 0);
}

void stop() {
	send(sock, "S", 1, 0);
}

static void hello( GtkWidget *widget,
                   gpointer   data ) {
    g_print ("Hello World\n");
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data ) {
    g_print ("delete event occurred\n");

    return TRUE;
}

static gboolean key_press_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data ) {
	guint keyval = ((GdkEventKey*)event)->keyval;
	guint32 time = ((GdkEventKey*)event)->time;

   	g_print ("[%d] key press event of (%s) [%d] occurred\n",time,((GdkEventKey*)event)->string,keyval);	

	/* if (keyval == 65362) { */
	/* 	go_forward(); */
	/* } else if (keyval == 65363) { // right */
	/* 	left_speed =- 3; */
	/* 	go_forward(); */
	/* } else if (keyval == 65361) { // left */
	/* 	right_speed =- 3; */
	/* 	go_forward(); */
	/* } */

    return TRUE;
}

static gboolean key_release_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data ) {
	guint keyval = ((GdkEventKey*)event)->keyval;
	guint32 time = ((GdkEventKey*)event)->time;
	GdkEvent *peeked_event;
	guint peeked_keyval = 0;
	guint peeked_time = 0;

	peeked_event = gdk_event_get();
	if (peeked_event != NULL && peeked_event->type == GDK_KEY_PRESS)  {
		//g_print("peeked a key\n");
		peeked_keyval = ((GdkEventKey*)peeked_event)->keyval;
		peeked_time = ((GdkEventKey*)peeked_event)->time;
		if (keyval == peeked_keyval) { //&& time == peeked_time)
			if (keyval == 65362) {
				left_speed +=6;
				right_speed +=6;
				go_forward();
			} else if (keyval == 65363) { //right 
				left_speed =- 3;
				go_forward();
			} else if (keyval == 65361) { //left 
				right_speed =- 3;
				go_forward();
			}
			return TRUE;			
		}
	}

	if (keyval == 65362) {
		left_speed=10;
		right_speed=10;
		stop();
	} else if (keyval == 65363) {
		left_speed=right_speed;
		go_forward();
	} else if (keyval == 65361) {
		right_speed=left_speed;
		go_forward();
	}

	g_print ("[%d] key release event of (%s) [%d] occurred.\n",time,((GdkEventKey*)event)->string,keyval);

    return TRUE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data ) {
    gtk_main_quit ();
}

// get sockaddr, IPv4 or IPv6:
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
        return 1;
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
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

	send(sockfd, "F1", 2, 0);
	sleep(1);
	send(sockfd, "S", 1, 0);

	return sockfd;
}

int main( int   argc,
          char *argv[] )
{
    /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
    GtkWidget *button;

	sock = roombad_connect();	
    
    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (&argc, &argv);

    /* create a new window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (G_OBJECT (window), "delete_event",
		      G_CALLBACK (delete_event), NULL);
    

	gtk_widget_set_events(window, GDK_KEY_PRESS_MASK); 
	gtk_widget_set_events(window, GDK_KEY_RELEASE_MASK); 

    /* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT (window), "destroy",
		      G_CALLBACK (destroy), NULL);

    g_signal_connect (G_OBJECT (window), "key_press_event",
		      G_CALLBACK (key_press_event), NULL);

    g_signal_connect (G_OBJECT (window), "key_release_event",
		      G_CALLBACK (key_release_event), NULL);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    
    /* Creates a new button with the label "Hello World". */
    button = gtk_button_new_with_label ("Hello World");
    
    /* When the button receives the "clicked" signal, it will call the
     * function hello() passing it NULL as its argument.  The hello()
     * function is defined above. */
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (hello), NULL);
    
    /* This will cause the window to be destroyed by calling
     * gtk_widget_destroy(window) when "clicked".  Again, the destroy
     * signal could come from here, or the window manager. */
    g_signal_connect_swapped (G_OBJECT (button), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (window));
    
    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), button);
    
    /* The final step is to display this newly created widget. */
    gtk_widget_show (button);
    
    /* and the window */
    gtk_widget_show (window);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
    
    return 0;
}
