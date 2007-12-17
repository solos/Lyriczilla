AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = lyriczilla-nest
lyriczilla_nest_SOURCES = nest.c ../gtk-lyricview/lyricview.h ../gtk-lyricview/lyricview.c ../lyriczilla/lz_dbus_client.c
lyriczilla_nest_CFLAGS = `pkg-config --cflags gtk+-2.0 audacious dbus-glib-1 dbus-1 libglade-2.0`
lyriczilla_nest_LDFLAGS = `pkg-config --libs gtk+-2.0 audacious libxml-2.0 dbus-glib-1 dbus-1 libglade-2.0`

