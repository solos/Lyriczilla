import lyriczilla.util
import ConfigParser

import os
import gtk.gdk

class Prefs:
	colors = {}
	font = 'Sans 12'
	
	def __init__(self, profile):
		print 'prefs init', profile
		
		self.colors['background'] = gtk.gdk.color_parse('black')
		self.colors['normal'] = gtk.gdk.color_parse('blue')
		self.colors['current'] = gtk.gdk.color_parse('white')
		self.colors['message'] = gtk.gdk.color_parse('yellow')
		
		home_dir = os.environ['HOME']
		self.filename = home_dir + '/.lyriczilla/config/%s.conf' % (profile)
		
		self.cf = ConfigParser.ConfigParser()
		self.cf.read(self.filename)
		
		if 'general' in self.cf.sections():
			self.font = self.cf.get("general", "font")
	
		if 'colors' in self.cf.sections():
			self.colors['background'] = gtk.gdk.color_parse(self.cf.get('colors', 'background'))
			self.colors['normal'] = gtk.gdk.color_parse(self.cf.get('colors', 'normal'))
			self.colors['current'] = gtk.gdk.color_parse(self.cf.get('colors', 'current'))
			self.colors['message'] = gtk.gdk.color_parse(self.cf.get('colors', 'message')) 
		
	def save(self):
		print self.cf.sections()
		if 'general' not in self.cf.sections():
			self.cf.add_section('general')
		self.cf.set('general', 'font', self.font)
		
		if 'colors' not in self.cf.sections():
			self.cf.add_section('colors')
		self.cf.set('colors', 'background', self.colors['background'].to_string())
		self.cf.set('colors', 'normal', self.colors['normal'].to_string())
		self.cf.set('colors', 'current', self.colors['current'].to_string())
		self.cf.set('colors', 'message', self.colors['message'].to_string())
		
		lyriczilla.util.makedirs_for_file(self.filename)
		self.cf.write(open(self.filename, 'w'))
		
	def __del__(self):
		print 'sdfjasdlfjlskadjflksajdlkfjsalkdfjlksadjfkl'
		self.save()

