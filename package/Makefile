
all:	deb

deb:	deb-lyriczilla deb-lyriczilla-plugin-bmp deb-lyriczilla-plugin-audacious

deb-lyriczilla:
	mkdir target
	mkdir -p target/DEBIAN target/usr/bin
	cp control/lyriczilla target/DEBIAN/control
	cp ../lyriczilla/lyriczilla ../lyriczilla/lrc2xml.py target/usr/bin
	dpkg -b target lyriczilla-0.0.3-all.deb
	rm -rf target

deb-lyriczilla-plugin-bmp:	../plugin-bmp
	mkdir target
	mkdir -p target/DEBIAN target/usr/lib/bmp/General
	cp control/lyriczilla-plugin-bmp target/DEBIAN/control
	cp ../plugin-bmp/lyriczilla-bmp.so target/usr/lib/bmp/General
	dpkg -b target lyriczilla-plugin-bmp-0.0.3-i386.deb
	rm -rf target

deb-lyriczilla-plugin-audacious:	../plugin-bmp
	mkdir target
	mkdir -p target/DEBIAN target/usr/lib/audacious/General
	cp control/lyriczilla-plugin-audacious target/DEBIAN/control
	cp ../plugin-bmp/lyriczilla-audacious.so target/usr/lib/audacious/General
	dpkg -b target lyriczilla-plugin-audacious-0.0.3-i386.deb
	rm -rf target

clean:
	rm -rf *.deb target
	
rebuild:
	make clean all

