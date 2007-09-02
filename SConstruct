# vim: set filetype: py

from build_util import Dev

opts = Options('custom.py', ARGUMENTS)

opts.AddOptions(
	EnumOption('tools', 'Toolset to compile with, default = platform default (msvc under windows)', 'mingw', ['mingw', 'default']),
	EnumOption('mode', 'Compile mode', 'debug', ['debug', 'release']),
	BoolOption('nativestl', 'Use native STL instead of STLPort', 'yes'),
	BoolOption('gch', 'Use GCH when compiling GUI (disable if you have linking problems with mingw)', 'yes'),
	BoolOption('verbose', 'Show verbose command lines', 'no'),
	BoolOption('savetemps', 'Save intermediate compilation files (assembly output)', 'no'),
	BoolOption('unicode', 'Build a Unicode version which fully supports international characters', 'yes'),
	('prefix', 'Prefix to use when cross compiling', 'i386-mingw32-')
)

gcc_flags = {
	'common': ['-g', '-Wall', '-Wextra', '-Wno-unused-parameter', '-Wno-missing-field-initializers', '-fexceptions', '-mthreads'],
	'debug': [], 
	'release' : ['-O2']
}

gcc_xxflags = {
	'common' : [],
	'debug' : [],
	'release' : ['-fno-enforce-eh-specs']
}

msvc_flags = {
	# 4512: assn not generated, 4100: <something annoying, forget which>, 4189: var init'd, unused, 4996: fn unsafe, use fn_s
	# 4121: alignment of member sensitive to packing
	'common' : ['/W4', '/EHsc', '/Zi', '/GR', '/wd4121', '/wd4100', '/wd4189', '/wd4996', '/wd4512'],
	'debug' : ['/MT'],
	'release' : ['/O2', '/MT']
}

msvc_xxflags = {
	'common' : [],
	'debug' : [],
	'release' : []
}

gcc_link_flags = {
	'common' : ['-g', '-Wl,--no-undefined', '-time'],
	'debug' : [],
	'release' : []				
}

msvc_link_flags = {
	'common' : ['/DEBUG', '/FIXED:NO', '/INCREMENTAL:NO', '/SUBSYSTEM:WINDOWS'],
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

tools = ARGUMENTS.get('tools', 'mingw')

toolset = [tools, 'swig']

env = Environment(tools = toolset, options=opts, ENV=os.environ)
Help(opts.GenerateHelpText(env))

if 'mingw' not in env['TOOLS'] and 'gcc' in env['TOOLS']:
	print "Non-mingw gcc builds not supported"
	Exit(1)

mode = env['mode']
if mode not in gcc_flags:
	print "Unknown mode, exiting"
	Exit(1)

dev = Dev(mode, tools, env)
dev.prepare()

env.SConsignFile()
env.Tool("gch", toolpath=".")

env.Append(CPPPATH = ["#/boost/boost/tr1/tr1/", "#/boost/", "#/htmlhelp/include/"])
env.Append(LIBPATH = ["#/htmlhelp/lib/"])

if not env['nativestl']:
	env.Append(CPPPATH = ['#/stlport/stlport/'])
	env.Append(LIBPATH = ['#/stlport/lib/'])
	env.Append(CPPDEFINES = ['HAVE_STLPORT', '_STLP_USE_STATIC_LIB=1'])
	if mode == 'debug':
		env.Append(LIBS = ['stlportg.5.1'])
	else:
		env.Append(LIBS = ['stlport.5.1'])	
elif 'gcc' in env['TOOLS']:
	env.Append(CPPDEFINES = ['BOOST_HAS_GCC_TR1'])
	
if env['savetemps'] and 'gcc' in env['TOOLS']:
	env.Append(CCFLAGS = ['-save-temps', '-fverbose-asm'])

if env['unicode']:
	env.Append(CPPDEFINES = ['UNICODE', '_UNICODE'])
	
if env['CC'] == 'cl':
	flags = msvc_flags
	xxflags = msvc_xxflags
	link_flags = msvc_link_flags
	defs = msvc_defs
	
	env.Append(LIBS = ['User32', 'shell32', 'Advapi32'])
else:
	flags = gcc_flags
	xxflags = gcc_xxflags
	link_flags = gcc_link_flags
	defs = gcc_defs

env.Append(CPPDEFINES = defs[mode])
env.Append(CPPDEFINES = defs['common'])

env.Append(CCFLAGS = flags[mode])
env.Append(CCFLAGS = flags['common'])

env.Append(LINKFLAGS = link_flags[mode])
env.Append(LINKFLAGS = link_flags['common'])

env.SourceCode('.', None)

import SCons.Scanner
SWIGScanner = SCons.Scanner.ClassicCPP(
	"SWIGScan",
	".i",
	"CPPPATH",
	'^[ \t]*[%,#][ \t]*(?:include|import)[ \t]*(<|")([^>"]+)(>|")'
)
env.Append(SCANNERS=[SWIGScanner])

from makedefs import convert
env.Command('dcpp/StringDefs.cpp', 'dcpp/StringDefs.h', lambda target, source, env: convert())
env.Depends('dcpp/StringDefs.cpp', 'dcpp/StringDefs.h')
env.SideEffect('Example.xml', 'dcpp/StringDefs.cpp')

dev.zlib = dev.build('zlib/')
dev.bzip2 = dev.build('bzip2/')
dev.boost = dev.build('boost/')
dev.yassl = dev.build('yassl/')
dev.client = dev.build('dcpp/')
env.Depends(dev.client, 'dcpp/StringDefs.cpp')
dev.smartwin = dev.build('smartwin/')
dev.win32 = dev.build('win32/')
