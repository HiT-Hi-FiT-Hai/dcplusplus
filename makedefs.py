#!/usr/bin/python

def makename(oldname):
	name = "";
	nextBig = True;
	for x in oldname:
		if x == '_':
			nextBig = True;
		else:
			if nextBig:
				name += x.upper();
				nextBig = False;
			else:
				name += x.lower();
				
	return name;

def convert():
	import re
	import codecs
	from xml.sax.saxutils import quoteattr, escape
	version = re.search("VERSIONSTRING (\S+)", file("dcpp/version.h").read()).group(1)
	
	varstr = "";
	strings = "";
	varname = "";
	names = "";
	
	prolog = "";
	epilog = "";
	
	example = '<?xml version="1.0" encoding="utf-8" standalone="yes"?>\n';
	example += '<Language Name="Example Language" Native="English" Code="en" Author="arnetheduck" Version=' + version + ' Revision="1" RightToLeft="0">\n'
	example += '\t<Strings>\n';
	
	lre = re.compile('\s*(\w+),\s*//\s*\"(.+)\"\s*')
	
	decoder = codecs.getdecoder('cp1252')
	encoder = codecs.getencoder('utf8')
	recodeattr = lambda s: encoder(decoder(quoteattr(s))[0])[0]
	recodeval = lambda s: encoder(decoder(escape(s, {"\\\\":"\\","\\t":"\t"}))[0])[0]
	
	for x in file("dcpp/StringDefs.h", "r"):
	    if x.startswith("// @Strings: "):
	        varstr = x[13:].strip();
	    elif x.startswith("// @Names: "):
	        varname = x[11:].strip();
	    elif x.startswith("// @Prolog: "):
	        prolog += x[12:];
	    elif x.startswith("// @Epilog: "):
	        epilog += x[12:];
	    elif len(x) >= 5:
	        match = lre.match(x);
	        if match is not None:
	            name , value = match.groups();
	            strings += '"' + value + '", \n'
	            newname = makename(name)
	            names += '"' + newname + '", \n'
	            example += '\t\t<String Name=%s>%s</String>\n' % (recodeattr(newname),  recodeval(value))
	
	example += '\t</Strings>\n';
	example += '</Language>\n';
	
	file('dcpp/StringDefs.cpp', 'w').write(prolog + varstr + " = {\n" + strings + "};\n" + varname + " = {\n" + names + "};\n" + epilog);
	file('Example.xml', 'w').write(example);

if __name__ == "__main__":
    convert()
