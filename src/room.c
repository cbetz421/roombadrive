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

static void
egg_room_face_init (EggRoomFace *room)
{
}

static void
draw (GtkWidget *room, cairo_t *cr)
{
	double x, y;
	double radius;
	int i;
	
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

	cairo_move_to(cr, 100, 100);
	cairo_line_to(cr, 200, 200);
	cairo_stroke (cr);

}


static gboolean
egg_room_face_expose (GtkWidget *room, GdkEventExpose *event)
{
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

GtkWidget *
egg_room_face_new (void)
{
	return g_object_new (EGG_TYPE_ROOM_FACE, NULL);
}
