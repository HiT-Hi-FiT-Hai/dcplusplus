import re
import codecs

def makename(n):
	newname = "";
	nextBig = True;
	for x in n:
		if x == '_':
			nextBig = True;
		else:
			if nextBig:
				newname += x.upper();
				nextBig = False;
			else:
				newname += x.lower();
				
	return newname;

def encval(string):
	return string.replace('<', '&lt;').replace('>', '&gt;').replace('&', '&amp;');
	
def encattr(string):
	return encval(string).replace('"', '&quot;').replace("'", '&apos;');
	
version = re.search("VERSIONSTRING (\S+)", file("client/version.h").read()).group(1)

varstr = "";
strings = "";
varname = "";
names = "";

prolog = "";

example = '<?xml version="1.0" encoding="utf-8" standalone="yes"?>\n';
example += '<Language Name="Example Language" Author="arnetheduck" Version=' + version + ' Revision="1">\n'
example += '\t<Strings>\r\n';

lre = re.compile('\s*(\w+),\s*//\s*\"(.+)\"\s*')
decoder = codecs.getdecoder('cp1252');
encoder = codecs.getencoder('utf8');
for x in file("client/StringDefs.h", "r"):
    if x[:13] == "// @Strings: ":
        varstr = x[13:].strip();
    elif x[:11] == "// @Names: ":
        varname = x[11:].strip();
    elif x[:12] == "// @Prolog: ":
        prolog += x[12:];
    elif len(x) >= 5:
        match = lre.match(x);
        if match is not None:
            name , value = match.groups();
            strings += '"' + value + '", \n';
            newname = makename(name)
            names += '"' + newname + '", \n';
            example += '\t\t<String Name="' + encoder(decoder(encattr(newname))[0])[0] + '">' + encoder(decoder(encval(value))[0])[0] + '</String>\n';

example += '\t</Strings>\n';
example += '</Language>\n';

file('client/StringDefs.cpp', 'w').write(prolog + varstr + " = {\n" + strings + "};\n" + varname + " = {\n" + names + "};\n");
file('Example.xml', 'w').write(example);