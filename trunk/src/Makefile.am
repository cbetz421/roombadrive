AM_CFLAGS = -I${top_srcdir}/libsrc $(NCURSES_CFLAGS) $(GTK2_CFLAGS)

if HAVE_GTK2
GTK2_PROGS=map_room
endif

bin_PROGRAMS = drive_ncurses $(GTK2_PROGS)

drive_ncurses_SOURCES= drive-ncurses.c
drive_ncurses_LDADD = $(GLIB2_LIBS) $(NCURSES_LIBS) $(top_builddir)/libsrc/libroomba.la 

map_room_SOURCES= map-room.c room.c room.h
map_room_LDADD = $(GTK2_LIBS) $(top_builddir)/libsrc/libroomba.la 
