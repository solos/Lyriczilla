
SVNVERSION=`svnversion -nc . | sed -e 's/^[^:]*://;s/[A-Za-z]//'`
VERSION=0.1.$(SVNVERSION)
DIR=lyriczilla gtk-lyricview plugin-bmp package

all:
	$(MAKE) version.h
	for dir in $(DIR); do \
		$(MAKE) -C $$dir; \
	done

version.h:
	echo "#define VERSION \"$(VERSION)\"" > $@

clean:
	@rm -f version.h
	@for dir in $(DIR); do \
		$(MAKE) -C $$dir clean; \
	done

