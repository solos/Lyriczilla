# Dbus service file
servicedir = $(datadir)/dbus-1/services
service_in_files = com.googlecode.LyricZilla.service.in
service_DATA = $(service_in_files:.service.in=.service)
CLEANFILES = com.googlecode.LyricZilla.service
EXTRA_DIST = com.googlecode.LyricZilla.service.in
# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files)
	@sed -e "s|__bindir__|$(bindir)|g" $< >$@


gladedir = $(datadir)/lyriczilla
glade_DATA = ui.glade
