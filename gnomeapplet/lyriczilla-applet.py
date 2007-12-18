#!/usr/bin/env python

import pygtk
pygtk.require('2.0')

import gtk
import gnomeapplet

import subprocess

def lyriczilla_applet_factory(applet, iid):
	socket = gtk.Socket()
	applet.add(socket)
	
	subprocess.Popen(['lyriczilla-nest', str(socket.get_id())])
	
	socket.set_size_request(400, 20)
	applet.show_all()
	return gtk.TRUE

gnomeapplet.bonobo_factory('OAFIID:LyricZilla_Applet_Factory', gnomeapplet.Applet.__gtype__, "LyricZillaApplet", "0", lyriczilla_applet_factory)
