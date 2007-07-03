CC=gcc
CFLAGS=-O2 -DAUDACIOUS `pkg-config --cflags gtk+-2.0 libxml-2.0`
CFLAGS_A=-O2 -DAUDACIOUS `pkg-config --cflags gtk+-2.0 audacious libxml-2.0`
CFLAGS_B=-O2 -DBEEP_MEDIA_PLAYER `pkg-config --cflags gtk+-2.0 bmp libxml-2.0`
LDFLAGS=-shared `pkg-config --libs gtk+-2.0 libxml-2.0`

all:	lyriczilla-bmp.so lyriczilla-audacious.so

lyriczilla-bmp.so: ../gtk-lyricview/lyricview.o lyriczilla-bmp.o
	$(CC) $(LDFLAGS) -o $@  $^

lyriczilla-audacious.so: ../gtk-lyricview/lyricview.o lyriczilla-audacious.o
	$(CC) $(LDFLAGS) -o $@  $^

lyriczilla-bmp.o: lyriczilla-bmp.c
	$(CC) $(CFLAGS_B) -o $@ -c $<

lyriczilla-audacious.o: lyriczilla-bmp.c
	$(CC) $(CFLAGS_A) -o $@ -c $<

install: all
	mkdir -p /usr/lib/bmp/General
	install -s -m 644 lyriczilla-bmp.so /usr/lib/bmp/General/
	mkdir -p /usr/lib/audacious/General
	install -s -m 644 lyriczilla-audacious.so /usr/lib/audacious/General/

uninstall:
	rm /usr/lib/bmp/General/lyriczilla-bmp.so
	rm /usr/lib/audacious/General/lyriczilla-audacious.so

clean:
	rm -f *.o *.so
	
rebuild:
	make clean all

