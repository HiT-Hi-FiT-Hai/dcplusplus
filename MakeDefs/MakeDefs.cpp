// MakeDefs.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../client/DCPlusPlus.h"

#include "../client/SimpleXML.h"
#include "../client/File.h"
#include "../client/StringTokenizer.h"

string Util::emptyString;
/*
int __cdecl main(int argc, char* argv[]) {
	File src(argv[1], File::READ, File::OPEN);
	string x = src.read();

	DWORD y = GetTickCount();
	SimpleXML xml;
	xml.fromXML(x);
	printf("%d\n", GetTickCount() - y);
	return 0;
}
*/
int __cdecl main(int argc, char* argv[])
{
	if(argc < 3) {
		return 0;
	}
	
	try {
		File src(argv[1], File::READ, File::OPEN);
		File tgt(argv[2], File::WRITE, File::CREATE | File::TRUNCATE);
		File example(argv[3], File::WRITE, File::CREATE | File::TRUNCATE);
		string x = src.read();

		string::size_type k;
		
		while((k = x.find('\r')) != string::npos) {
			x.erase(k, 1);
		}

		StringList l = StringTokenizer(x).getTokens();

		StringIter i;
		string varStr;
		string varName;
		string start;

		SimpleXML ex;

		for(i = l.begin(); i != l.end(); ) {
			if( (k = i->find("// @Strings: ")) != string::npos) {
				varStr = i->substr(k + 13);
				i = l.erase(i);
			} else if( (k = i->find("// @Names: ")) != string::npos) {
				varName = i->substr(k + 11);
				i = l.erase(i);
			} else if(i->find("// @DontAdd") != string::npos) {
				i = l.erase(i);
			} else if( (k = i->find("// @Prolog: ")) != string::npos) {
				start += i->substr(k + 12) + "\r\n";
				i = l.erase(i);
			} else if(i->size() < 5) {
				i = l.erase(i);
			} else {
				++i;
			}
		}

		if(varStr.empty() || varName.empty()) {
			printf("No @Strings or @Names\n");
			return 0;
		}
		
		varStr += " = {\r\n\t";
		varName += " = {\r\n\t";
		
		ex.addTag("Language");
		ex.addChildAttrib("Name", string("Example Language"));
		ex.stepIn();
		ex.addTag("Strings");
		ex.stepIn();
		int a = 0;
		for(i = l.begin(); i != l.end(); i++) {
			string name;
			string def;

			string s = *i;

			
			bool u = true;
			for(k = s.find_first_not_of(" \t"); s[k] != ','; k++) {
				if(s[k] == '_') {
					u = true;
				} else if(u) {
					name+=s[k];
					u = false;
				} else {
					name+=(char)tolower(s[k]);
				}
			}

			k = s.find("// ");
			def = s.substr(k + 3);
			
			ex.addTag("String", def.substr(1, def.size() - 2));
			ex.addChildAttrib("Name", name);

			varStr += def + ", ";
			varName += '\"' + name + "\", ";

			if(((++a) % 10) == 0) {
				varStr += "\r\n";
				varName += "\r\n";
			}

		}

		varStr.erase(varStr.size()-2, 2);
		varName.erase(varName.size()-2, 2);

		varStr += "\r\n};\r\n";
		varName += "\r\n};\r\n";

		tgt.write(start);
		tgt.write(varStr);
		tgt.write(varName);

		example.write("<?xml version=\"1.0\" encoding=\"windows-1252\"?>\r\n");
		example.write(ex.toXML());
	} catch(Exception e) {
		printf("%s\n", e.getError().c_str());
	}

	return 0;
}
