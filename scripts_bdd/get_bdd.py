#!/usr/bin/python
# -*- coding: utf-8 -*-
import urllib2, re
from BeautifulSoup import BeautifulSoup
########################################################################
def GetFilesnames(url):
	s = urllib2.urlopen(url).read()
	soup = s.split("\n")	
	
	filename =""
	description =""
	source =""
	date =""
	
	OK = 0
	
	for line in soup:
		if line.find('<th class="cz-description">Description</th>') > 0:
			OK = 1
		if (OK == 1):	
			if line.find('<td class="cz-name">') > 0:
				description = line.split('>')[2].split('<')[0]
				if description.find('&') != -1 or description.find('[') != -1:
					description = ""
			elif line.find('<td class="cz-filename">') > 0:
				filename = line.split('>')[2].split('<')[0]
			elif line.find('<td class="cz-description">') > 0:
				#have src or not
				if line.find('<a href=') > 0:
					source = line.split('"')[3]
					if source.find('docid=') > 0:
						date = source.split('docid=')[1].split('-')[0]+"/"+source.split('docid=')[1].split('-')[1][0]+source.split('docid=')[1].split('-')[1][1]+"/"+source.split('docid=')[1].split('-')[1][2]+source.split('docid=')[1].split('-')[1][3]
						print '"'+filename+'";"";"'+description+': '+line.split('>')[2].split('<')[0]+', '+line.split('>')[3].split('<')[0]+'";"'+source+'";"'+date+'"'
					else:
						print '"'+filename+'";"";"'+description+': '+line.split('>')[2].split('<')[0]+', '+line.split('>')[3].split('<')[0]+'";"'+source+'";""'
				else:
					print '"'+filename+'";"";"'+description+': '+line.split('>')[1].split('<')[0]+'";"";""'
			
########################################################################
GetFilesnames("http://www.bleepingcomputer.com/startups/rootkits/")

i = 2
while (i < 1000):
	url = "http://www.bleepingcomputer.com/startups/rootkits/page/"+str(i)+"/"
	GetFilesnames(url)
	i=i+1


