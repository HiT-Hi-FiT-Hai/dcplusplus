#!/usr/bin/env python
import os, re, cgi

filetext = "../changelog.txt"
filehtml = "changelog.html"

#os.rename(filehtml,"%s.old" % filehtml)

fp_txt = open(filetext,'r')
fp_html = open(filehtml,'w')

start_head = """<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<head>
  <meta content=\"en-us\" http-equiv=\"Content-Language\">
  <meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">
  <link href=\"office11.css\" rel=\"stylesheet\" type=\"text/css\">
  <title>Changelog</title>
  <style type=\"text/css\">
    li { margin-left: auto; margin: 0em 0em 0em 0em; }
  </style>
</head>
<body>
<h1>DC++ Changelog</h1>
See the version history of DC++ below.

"""

end_html = "</body>\n</html>"

start_change = "  <li>%(change)s"
bug_text = "  <li><a href=\"http://dcpp.net/bugzilla/show_bug.cgi?id=%(bug_id)s\">[bug %(bug_id)s]</a> %(change)s"
change = " %(change)s"
end_change = "</li>\n"

start_version = "<h2>%(version)s <span style=\"color: gray;\">(%(date)s)</span></h2>\n<ul>\n"
end_version = "</ul>\n\n"

start_warning_end = "  <li><span style=\"color: red;\">%(change)s</span></li>\n"

new_version_pattern = re.compile("^.*?-- (?P<version>.*?) (?P<date>.*?) --")
new_change = re.compile(r"^\* (?P<change>.*?)$")
bug_change = re.compile(r"^\* \[bug (?P<bug_id>\d+?)\] (?P<change>.*?)$")
continue_change = re.compile("^\w*?(?P<change>.*?)$")
warning_change = re.compile("^(?P<change>[^ ].*?)$")
fp_html.write(start_head)

open_change_state = False
close_version = False
start = False

for line in fp_txt:
	line = cgi.escape(line.strip())
	if not line:
		if open_change_state:
			fp_html.write(end_change)
			open_change_state = False
		continue
		
	mObj = new_version_pattern.match(line)
	if mObj and mObj.groupdict()["date"] :
		if close_version:
			if open_change_state:
				fp_html.write(end_change)
			fp_html.write(end_version)
		start = True
		close_version = True
		open_change_state = False
		open_warning_state = False
		fp_html.write(start_version % mObj.groupdict())
		continue
	
	if not start:
		continue
	
	mObj = bug_change.match(line)
	if mObj:
		if open_change_state:
		    fp_html.write(end_change)
		fp_html.write(bug_text % mObj.groupdict())
		open_change_state = True
		continue 

	mObj = new_change.match(line)
	if mObj: # A new change is found: Close Open Warning or Changes.
		if open_change_state:
			fp_html.write(end_change)
		fp_html.write(start_change % mObj.groupdict())
		open_change_state = True
		continue 
	
	mObj = continue_change.match(line)
	if mObj and open_change_state: #A continutaion of an change (multiline change).
		fp_html.write(change % mObj.groupdict())
		continue
		
	mObj = warning_change.match(line)
	if mObj:
		if open_change_state:
			fp_html.write(end_change_state)
		fp_html.write(start_warning_end % mObj.groupdict()) 
		continue

if open_change_state:
	fp_html.write(end_change)
if close_version:
	fp_html.write(end_version)
fp_html.write(end_html)
fp_html.close()
fp_txt.close()
