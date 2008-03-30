#this script browses through every HTML files in the current directory, looking for tags that have the "cshelp" argument; cshelp.h and cshelp.txt files are then generated and used by the DC++ context-sensitive help system

import glob
from HTMLParser import HTMLParser
from htmlentitydefs import entitydefs

#will hold [id, text] pairs
output = []

#define our HTML parsing class derived from HTMLParser
class Parser(HTMLParser):
    text = ""
    current_tag = ""
    count = 0 #to handle sub-tags with the same name as the current tag; eg <x cshelp="y">bla <x>bla</x> bla</x>

    def handle_starttag(self, tag, attrs):
        if self.count > 0:
            if tag == "a":
                #enclose links with quotes so it looks fancier
                self.text += "\""
            if tag == self.current_tag:
                self.count += 1
            return

        #attrs is a list of tuples; each tuple being an (attr-name, attr-content) pair
        for attr in attrs:
            if attr[0] == "cshelp":
                output.append([attr[1]])
                self.current_tag = tag
                self.count += 1

    def handle_data(self, data):
        if self.count > 0:
            self.text += data

    def handle_entityref(self, name):
        if self.count > 0:
            self.text += entitydefs[name]

    def handle_endtag(self, tag):
        if self.count > 0:
            if tag == "a":
                #enclose links with quotes so it looks fancier
                self.text += "\""
            if tag == self.current_tag:
                self.count -= 1
                if self.count == 0:
                    #reached the end of the current tag
                    output[-1].append(self.text.replace("\r", " ").replace("\n", " ").strip().replace("  ", " "))
                    self.text = ""
                    self.current_tag = ""

#parse all HTML files in the current folder
glob = glob.glob("*.html")
for html_file in glob:
    parser = Parser()
    f = open(html_file, "r")
    parser.feed(f.read())
    f.close()
    parser.close()

#generate cshelp.h and cshelp.txt
f_h = open("cshelp.h", "w")
f_h.write("""// this file contains help ids for field-level help tooltips

#ifndef DCPLUSPLUS_HELP_CSHELP_H
#define DCPLUSPLUS_HELP_CSHELP_H

""")
number = 11000
f_txt = open("cshelp.txt", "w")
for entry in output:
    f_h.write("#define " + entry[0] + " " + str(number) + "\r\n")
    number += 1
    f_txt.write(".topic " + entry[0] + "\r\n" + entry[1] + "\r\n")
f_h.write("""
#endif
""")
f_h.close()
f_txt.close()
