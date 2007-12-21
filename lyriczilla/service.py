#!/usr/bin/python

import os
import sys
import urllib2
import gettext
import md5
from optparse import OptionParser
from xml.dom import minidom
from base64 import b64encode, b64decode
import cPickle
import lyriczilla.util

def detect_charset(s):
	charsets = ('iso-8859-1', 'gbk', 'utf-8')
	for charset in charsets:
		try:
			return unicode(unicode(s, 'utf-8').encode(charset), 'gbk')
		except:
			continue
	return s

def remove_garbage_chars(s):
	rall = range(0x30, 0x3a) + range(0x41, 0x5b) + range(0x61, 0x7b) + range(0x4e00, 0x9fa6)
	t = ''
	nest = 0
	for ch in s:
		if ord(ch) in rall and nest == 0:
			t += ch
		elif ch == '(':
			nest += 1
		elif ch == ')':
			nest -= 1
		
	return t

def __get_lyric_list(title, artist):
	title_encode = urllib2.quote(remove_garbage_chars(detect_charset(title)).encode('gbk'))
	artist_encode = urllib2.quote(remove_garbage_chars(detect_charset(artist)).encode('gbk'))
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
	

def __get_lyric_source(url):
	return urllib2.urlopen(urllib2.quote(url.encode('gbk'), ':/')).read().decode('gbk')
	


def get_lyric_list(cacheable, uri, title, artist):
	print 'get_lyric_list', cacheable, uri, title, artist
	home_dir = os.environ['HOME']
	
	# if i found a local lrc file, load it first.
	#files = (
	#	home_dir + '/.lyriczilla/lyric/' + '%s - %s.lrc' % (title, artist),
		
	cache_filename = home_dir + '/.lyriczilla/cache/list/%s - %s.list' % (artist, title)
	l = None
	if cacheable:
		l = lyriczilla.util.load_object(list, cache_filename)
	
	if l != None:
		return l
	else:
		l = __get_lyric_list(title, artist)
		lyriczilla.util.dump_object(l, cache_filename)
		return l

def get_lyric(cacheable, url):
	home_dir = os.environ['HOME']
	cache_filename = home_dir + '/.lyriczilla/cache/lyric/' + md5.md5(url.encode('utf-8')).hexdigest() + '.lrc'
	
	lrc_text = None
	if cacheable:
		lrc_text = lyriczilla.util.load_text(cache_filename)
		
	if lrc_text == None:
		try:
			lrc_text = __get_lyric_source(url)
			lyriczilla.util.dump_text(lrc_text, cache_filename)
		except:
			pass
			
	if lrc_text != None:
		return lyriczilla.util.lrctolist(lrc_text)
	else:
		return list()

	
