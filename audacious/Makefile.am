AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = lyriczilla-aud.so
lyriczilla_aud_so_SOURCES = lyriczilla-aud.c ../gtk-lyricview/lyricview.c
lyriczilla_aud_so_CFLAGS = `pkg-config --cflags gtk+-2.0 audacious libxml-2.0`
lyriczilla_aud_so_LDFLAGS = -shared `pkg-config --libs gtk+-2.0 audacious libxml-2.0`
bindir = $(libdir)/audacious/General

