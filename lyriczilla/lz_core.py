#!/usr/bin/python

import sys
import urllib2
import gettext
from optparse import OptionParser
from xml.dom import minidom
from base64 import b64encode, b64decode
import cPickle

def detect_charset(s):
	charsets = ('iso-8859-1', 'gbk', 'utf-8')
	for charset in charsets:
		try:
			return unicode(unicode(s, 'utf-8').encode(charset), 'gbk')
		except:
			continue
	return s

def get_lyric_list(title, artist):

	title_encode = urllib2.quote(detect_charset(title).encode('gbk').replace(' ', ''))
	artist_encode = urllib2.quote(detect_charset(artist).encode('gbk').replace(' ',''))
	url = 'http://www.winampcn.com/lyrictransfer/get.aspx?song=%s&artist=%s&lsong=%s&Datetime=20060601' % (title_encode, artist_encode, title_encode)
	
	xmltext = urllib2.urlopen(url).read()
	try:
		xmltext = xmltext.decode('gbk').encode('UTF-8')
		xmltext = xmltext.replace('encoding="gb2312"', 'encoding="UTF-8"')
	except:
		pass

	xmldoc = minidom.parseString(xmltext)
	
	ret = []
	root = xmldoc.documentElement

	for node in root.getElementsByTagName('LyricUrl'):
		got_title = node.attributes['SongName'].value
		got_artist = node.attributes['Artist'].value
		got_url = node.childNodes[0].data
		ret.append({'url': got_url, 'title': got_title, 'artist': got_artist})
		
	return ret
	
def get_lyric_source(url):
	return urllib2.urlopen(urllib2.quote(url.encode('gbk'), ':/')).read().decode('gbk')
	
