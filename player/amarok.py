import lyriczilla.player
import commands

def amarok_get_info():
	(status, title) = commands.getstatusoutput('dcop amarok player title')
	if status: return None
	(status, artist) = commands.getstatusoutput('dcop amarok player artist')
	if status: return None
	(status, filename) = commands.getstatusoutput('dcop amarok player path')
	if status: return None
	(status, time) = commands.getstatusoutput('dcop amarok player trackCurrentTimeMs')
	if status: return None
	return {'title': title, 'artist': artist, 'filename': filename, 'time': time}
	
lyriczilla.player.register('Amarok', amarok_get_info)

