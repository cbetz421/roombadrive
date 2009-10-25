/**
 * room.h
 *
 * A GTK+ widget that implements a room face
 *
 * (c) 2005, Davyd Madeley
 *
 * Authors:
 *   Davyd Madeley  <davyd@madeley.id.au>
 */

#ifndef __EGG_ROOM_FACE_H__
#define __EGG_ROOM_FACE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_ROOM_FACE		(egg_room_face_get_type ())
#define EGG_ROOM_FACE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_ROOM_FACE, EggRoomFace))
#define EGG_ROOM_FACE_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), EGG_ROOM_FACE, EggRoomFaceClass))
#define EGG_IS_ROOM_FACE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_ROOM_FACE))
#define EGG_IS_ROOM_FACE_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), EFF_TYPE_ROOM_FACE))
#define EGG_ROOM_FACE_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_ROOM_FACE, EggRoomFaceClass))

typedef struct _EggRoomFace		EggRoomFace;
typedef struct _EggRoomFaceClass	EggRoomFaceClass;

struct _EggRoomFace
{
	GtkDrawingArea parent;

	/* < private > */
};

struct _EggRoomFaceClass
{
	GtkDrawingAreaClass parent_class;
};

GtkWidget *egg_room_face_new (void);

G_END_DECLS

#endif
