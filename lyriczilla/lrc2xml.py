#!/usr/bin/python

from sys import stdin
import re
from re import findall
from string import replace, lower
from xml.dom import minidom

def lrc2xml(lrc):
	#pattern = '\[(?P<type>.*?):(?P<value>.*?)\](?P<tail>.*)'
	pattern = '\[.*\](?P<tail>.?)'
	pt_value= '\[(?P<type>.+?):(?P<value>.+?)\]'
	rpl_tail='\g<tail>'

	lrc=lrc.splitlines()
	title=''
	artist=''
	album=''
	editor=''
	offset=0
	arrays=[]
	for line in lrc:
		line=replace(line,"\n","")
		line=replace(line,"\r","")
		if line=="":
			continue
		if line[0]=='[' and line!="": #if a CH char start with [ may crash

			value = findall(pt_value,line)
			ly = re.sub(pattern,rpl_tail,line)
			if value!=[]:
				name = lower(value[0][0])
				namev = value[0][1]
			
				if name=="ti":
					title = namev
					continue
				if name=='ar':
					artist = namev
					continue
				if name=='al':
					album = namev
					continue
				if name=='by':
					editor = namev
					continue
				if name=='offset':
					offset = long(namev)
					continue
				if name=='la':
					continue			
				for j in range(len(value)):
					time=offset #offset using here
					if value[j][1].find(".")!=-1:
						time+=int(value[j][0])*60*1000
						time+=int(value[j][1].split('.',2)[0])*1000+int(value[j][1].split('.',2)[1])
					else:
						time+=int(value[j][0])*60*1000
						time+=int(value[j][1])*1000
					if time < 0:
						time = 0;
					arrays.append((time,ly))

						
	arrays.sort()
	
	
	dom = minidom.getDOMImplementation().createDocument(None, 'song', None)
	root = dom.documentElement
	root.setAttribute('title', title);
	root.setAttribute('artist', artist);
	root.setAttribute('album', album);
	root.setAttribute('editor', editor);

	for time, text in arrays:
		elem = dom.createElement('one')
		elem.setAttribute('time', str(time))
		elem.appendChild(dom.createTextNode(text))
		root.appendChild(elem)
	return dom.toxml()

if __name__ == "__main__":
	print lrc2xml(stdin.read())
