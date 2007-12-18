import lyriczilla.player
import lyriczilla.util
import dbus

def rhythmbox_get_info():
	if not lyriczilla.util.dbus_name_running('org.gnome.Rhythmbox'): return None
	try:
		bus = dbus.SessionBus()
		player = dbus.Interface(bus.get_object('org.gnome.Rhythmbox', '/org/gnome/Rhythmbox/Player'), 'org.gnome.Rhythmbox.Player')
		shell = dbus.Interface(bus.get_object('org.gnome.Rhythmbox', '/org/gnome/Rhythmbox/Shell'), 'org.gnome.Rhythmbox.Shell')

		uri =  player.getPlayingUri()
		info = shell.getSongProperties(uri)

		return {'title': info['title'], 'artist': info['artist'], 'filename': uri, 'time': player.getElapsed()}
	except:
		return None
	
lyriczilla.player.register('Rhythmbox', rhythmbox_get_info)

