copy /b app\dcplusplus.exe .
copy /b app\dcplusplus.chm .
copy /b app\dcplusplus.pdb .
"C:\Program Files\7-Zip\7z.exe" a -tzip -mx9 DCPlusPlus-%1.zip dcppboot.xml unicows.dll unicows.pdb dcplusplus.chm dcplusplus.exe dcplusplus.pdb dbghelp.dll changelog.txt Example.xml License.txt GeoIPCountryWhois.csv
"C:\Program Files\7-Zip\7z.exe" a -tzip -mx9 DCPlusPlus-%1-src.zip dcppboot.xml makedefs.py libunicows.lib unicows.dll unicows.pdb yassl\* yassl\certs\* yassl\include\openssl\* yassl\mySTL\* yassl\src\* yassl\include\* yassl\taocrypt\* yassl\taocrypt\src\* yassl\taocrypt\include\* stlport\.keep wtl\.keep help\* zlib\* bzip2\* client\* res\* windows\* changelog.txt Example.xml License.txt compile.txt GeoIPCountryWhois.csv *.vcproj *.sln *.rc extensions.txt doxyfile dcplusplus.nsi -xr!.svn -xr!*.user
del dcplusplus.exe
del dcplusplus.pdb
del dcplusplus.chm
makensis dcplusplus.nsi
move dcplusplus.exe DCPlusPlus-%1.exe

svn status
pause
svn copy . https://svn.sourceforge.net/svnroot/dcplusplus/dcplusplus/tags/dcplusplus-%1 -m "%1"
