AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = lyriczilla-aud.so
lyriczilla_aud_so_SOURCES = lyriczilla-aud.c ../gtk-lyricview/lyricview.h ../gtk-lyricview/lyricview.c ../lyriczilla/lz_dbus_client.c
lyriczilla_aud_so_CFLAGS = `pkg-config --cflags gtk+-2.0 audacious dbus-glib-1 dbus-1`
lyriczilla_aud_so_LDFLAGS = -shared `pkg-config --libs gtk+-2.0 audacious libxml-2.0 dbus-glib-1 dbus-1`
bindir = $(libdir)/audacious/General

