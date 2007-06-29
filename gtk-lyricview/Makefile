CC=gcc
CFLAGS=`pkg-config --cflags gtk+-2.0 bmp libxml-2.0`
LDFLAGS=`pkg-config --libs gtk+-2.0 libxml-2.0`

all:	lyricview.o

lyricview.o: lyricview.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o 
	
rebuild:
	make clean all

