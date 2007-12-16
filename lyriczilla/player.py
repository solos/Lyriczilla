import sys
import glob
import os.path
import imp

players = []

def register(name, func):
	players.append((name, func))

def __scan():
	del players[:]

	scandirs = ['/home/acmadmin/lz-new/lyriczilla/player']

	for scandir in scandirs:
		print scandir
		try: names = glob.glob(os.path.join(scandir, "[!_]*.py"))
		except OSError: continue

		for pathname in names:
			name = os.path.basename(pathname)
			name = name[:name.rfind(".")]

		        try:
				sys.path.insert(0, scandir)
		           
				try: modinfo = imp.find_module(name)
				except ImportError: continue
				try:
					mod = imp.load_module(name, *modinfo)
				except Exception, err:
		                        try: del sys.modules[name]
		                        except KeyError: pass
			finally:
		            del sys.path[0:1]
		 
	return players

def get_info():
	for player in players:
		info = player[1]()
		if info != None: return info
	return None

