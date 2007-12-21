import lyriczilla.player
import dbus

def banshee_get_info():
	if not lyriczilla.util.dbus_name_running('org.gnome.Banshee'): return None
	try:
		player = dbus.SessionBus().get_object('org.gnome.Banshee', '/org/gnome/Banshee/Player')
		title = player.GetPlayingTitle()
		artist = player.GetPlayingArtist()
		uri = player.GetPlayingUri()
		time = player.GetPlayingPosition()
		return {'title': title, 'artist': artist, 'uri': uri, 'time': time * 1000}
	except:
		return None
	
lyriczilla.player.register('Banshee', banshee_get_info)

