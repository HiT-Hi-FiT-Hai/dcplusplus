# vim: set filetype: py

from build_util import Dev

gcc_flags = {
	'common': ['-ggdb', '-Wall', '-Wextra', '-Wno-unused-parameter', '-pipe', '-fexceptions'],
	'debug': [], 
	'release' : ['-O2']
}

msvc_flags = {
	'common' : ['/W4', '/EHsc', '/Zi', '/GR', '/wd4100'],
	'debug' : ['/MD'],
	'release' : ['/O2', '/MD']
}

gcc_link_flags = {
	'common' : ['-ggdb', '-Wl,--no-undefined'],
	'debug' : [],
	'release' : []				
}

msvc_link_flags = {
	'common' : ['/DEBUG', '/FIXED:NO', '/INCREMENTAL:NO'],
	'debug' : [],
	'release' : []
}

msvc_defs = {
	'common' : ['_REENTRANT', 'USE_SYS_STL=1'],
	'debug' : [''],
	'release' : ['NDEBUG']
}

gcc_defs = {
	'common' : ['_REENTRANT', 'USE_SYS_STL=1'],
	'debug' : ['_DEBUG'],
	'release' : ['NDEBUG']
}

# --- cut ---

import os,sys

if sys.platform == 'win32':
	tooldef = 'mingw'
else:
	tooldef = 'default'

mode = ARGUMENTS.get('mode', 'debug')
tools = ARGUMENTS.get('tools', tooldef)

if mode not in gcc_flags:
	print "Unknown mode, exiting"
	Exit(1)

toolset = [tools, 'swig']

env = Environment(tools = toolset, ENV=os.environ)

dev = Dev(mode, tools, env)
dev.prepare()

env.SConsignFile()
env.Tool("gch", toolpath=".")

if 'mingw' not in env['TOOLS'] and 'gcc' in env['TOOLS']:
	env.Append(CCFLAGS=['-fvisibility=hidden'])

if 'mingw' in env['TOOLS']:
	env.Append(CPPPATH = ['#/stlport/stlport/'])
	env.Append(LIBPATH = ['#/stlport/lib/'])
	env.Append(CPPDEFINES = ['HAVE_STLPORT', '_STLP_USE_STATIC_LIB=1'])
	
	if mode == 'debug':
		env.Append(LIBS = ['stlportg.5.1'])
	else:
		env.Append(LIBS = ['stlport.5.1'])	

if env['CC'] == 'cl':
	flags = msvc_flags
	link_flags = msvc_link_flags
	defs = msvc_defs
	
	# This is for msvc8
	# Embed generated manifest in file
	env['SHLINKCOM'] = [env['SHLINKCOM'], 'mt.exe -manifest ${TARGET}.manifest -outputresource:$TARGET;2']
	env['LINKCOM'] = [env['LINKCOM'], 'mt.exe -manifest ${TARGET}.manifest -outputresource:$TARGET;1']
else:
	flags = gcc_flags
	link_flags = gcc_link_flags
	defs = gcc_defs

env.Append(CPPDEFINES = defs[mode])
env.Append(CPPDEFINES = defs['common'])

env.Append(CCFLAGS = flags[mode])
env.Append(CCFLAGS = flags['common'])

env.Append(LINKFLAGS = link_flags[mode])
env.Append(LINKFLAGS = link_flags['common'])

env.SourceCode('.', None)
env.SetOption('implicit_cache', '1')
env.SetOption('max_drift', 60*10)

import SCons.Scanner
SWIGScanner = SCons.Scanner.ClassicCPP(
	"SWIGScan",
	".i",
	"CPPPATH",
	'^[ \t]*[%,#][ \t]*(?:include|import)[ \t]*(<|")([^>"]+)(>|")'
)
env.Append(SCANNERS=[SWIGScanner])

dev.zlib = dev.build('zlib/')
dev.bzip2 = dev.build('bzip2/')
dev.yassl = dev.build('yassl/')
dev.client = dev.build('client/')
dev.smartwin = dev.build('smartwin/')
dev.win32 = dev.build('win32/')
