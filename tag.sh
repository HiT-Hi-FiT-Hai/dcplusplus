#!/bin/sh

svn copy . https://dcplusplus.svn.sourceforge.net/svnroot/dcplusplus/dcplusplus/tags/dcplusplus-$1 -m "$1"
