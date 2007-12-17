import lyriczilla.player
import dbus

def quod_get_info():
	try:
		daemon = dbus.SessionBus().get_object('org.gnome.Listen', '/org/gnome/Listen')
		
		cpstr = daemon.current_playing()
		
		return None
	except:
		return None
	
lyriczilla.player.register('Quod Libet', quod_get_info)

