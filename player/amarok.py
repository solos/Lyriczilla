import commands

class amarok(lyriczilla.player):
	def __init__():
		pass
		
	def get_current_song():
		title = commands.getoutput('dcop amarok player title')
		artist = commands.getoutput('dcop amarok player artist')
		filename = commands.getoutput('dcop amarok player path')
		time = commands.getoutput('dcop amarok player trackCurrentTimeMs')

		return ('title': title, 'artist': artist, 'filename': filename, 'time': time)
	
	

def test():
	print title, artist, filename, time
