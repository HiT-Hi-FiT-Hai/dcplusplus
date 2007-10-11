#!/bin/bash

scons mode=release
cp build/release-mingw/bin/DCPlusPlus.exe .
i686-mingw32-strip DCPlusPlus.exe

#copy /b app\dcplusplus.chm .
zip -9 DCPlusPlus-$1.zip changelog.txt dcppboot.xml DCPlusPlus.chm DCPlusPlus.exe Example.xml License.txt LICENSE-GeoIP.txt magnet.exe GeoIPCountryWhois.csv

find *.txt *.TXT *.nsi *.xml *.py *.csv *.sh magnet.exe Doxyfile SConstruct boost bzip2 dcpp help htmlhelp res smartwin win32 yassl zlib \
 -wholename "*/.svn" -prune -o -print | grep -v gch$ | zip -9 DCPlusPlus-$1-src.zip -9 -@

makensis DCPlusPlus.nsi
mv DCPlusPlus-xxx.exe DCPlusPlus-$1.exe

rm DCPlusPlus.exe

svn status
read -p "Press ctrl-c to abort svn copy"
svn copy . https://svn.sourceforge.net/svnroot/dcplusplus/dcplusplus/tags/dcplusplus-$1 -m "$1"

