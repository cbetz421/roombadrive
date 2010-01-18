/**
 * room.c
 *
 * A GTK+ widget that implements a room face
 *
 * (c) 2005, Davyd Madeley
 *
 * Authors:
 *   Davyd Madeley  <davyd@madeley.id.au>
 */

#include <gtk/gtk.h>
#include <math.h>

#include "room.h"

G_DEFINE_TYPE (EggRoomFace, egg_room_face, GTK_TYPE_DRAWING_AREA);

static gboolean egg_room_face_expose (GtkWidget *room, GdkEventExpose *event);

static void
egg_room_face_class_init (EggRoomFaceClass *class)
{
	GtkWidgetClass *widget_class;

	widget_class = GTK_WIDGET_CLASS (class);

	widget_class->expose_event = egg_room_face_expose;
}

static void egg_room_face_init (EggRoomFace *room) {
	room->distance = 0;
}

static void draw (GtkWidget *room, cairo_t *cr) {
	double x, y;
	double radius;
	int i;
	float spot_x, spot_y;
	float speed_x = 1, speed_y = 1;
	GSList *list = EGG_ROOM_FACE(room)->events;
	
	x = room->allocation.x + room->allocation.width / 2;
	y = room->allocation.y + room->allocation.height / 2;
	radius = MIN (room->allocation.width / 2,
		      room->allocation.height / 2) - 5;

	/* room back */
//	cairo_arc (cr, x, y, radius, 0, 2 * M_PI);
	cairo_rectangle(cr, 0, 0, 500, 500);

	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_stroke (cr);

	cairo_move_to(cr, 250, 250);
	spot_x = 250;
	spot_y = 250;
	if(list != NULL) {
		do {
			struct _event *e = list->data;
			if(e->type == 'S') {
				spot_x -= e->l1 * speed_x *.25;
				spot_y -= e->l1 * speed_y *.25;
			} else if(e->type == 'W') {
				e->l1 = ABS(e->l1);
				e->l2 = ABS(e->l2);
				speed_x = (float)e->l1/1000.0;
				speed_x = 0;
				speed_y = (float)e->l2/1000.0;
				printf(":: [%d]\n", e->l2);
			}
			
			//printf("MOVE!! [%f][%f]\n", spot_x, spot_y);
			cairo_line_to(cr, spot_x, spot_y);
		} while((list = g_slist_next(list)) != NULL);
	}
//HERE
//	cairo_line_to(cr, 250, 250 - EGG_ROOM_FACE(room)->distance);
	cairo_stroke (cr);

}


static gboolean egg_room_face_expose (GtkWidget *room, GdkEventExpose *event) {
	cairo_t *cr;

	/* get a cairo_t */
	cr = gdk_cairo_create (room->window);

	cairo_rectangle (cr,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
	cairo_clip (cr);
	
	draw (room, cr);

	cairo_destroy (cr);

	return FALSE;
}

void egg_room_face_redraw_canvas (GtkWidget *widget) {
	GdkRegion *region;
	
	if (!widget->window) return;

	region = gdk_drawable_get_clip_region (widget->window);
	/* redraw the cairo canvas completely by exposing it */
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_window_process_updates (widget->window, TRUE);

	gdk_region_destroy (region);
}

void egg_room_face_set_distance(GtkWidget *widget, long distance) {
	EGG_ROOM_FACE(widget)->distance = distance;
}

void egg_room_face_add_event (GtkWidget *widget, struct _event *event) {

	EGG_ROOM_FACE(widget)->events = g_slist_append(EGG_ROOM_FACE(widget)->events, g_memdup(event, sizeof(struct _event)));
}

GtkWidget *egg_room_face_new (void) {
	return g_object_new (EGG_TYPE_ROOM_FACE, NULL);
}
