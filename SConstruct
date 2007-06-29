# vim: set filetype: py

from build_util import Dev

opts = Options('custom.py', ARGUMENTS)

opts.AddOptions(
	EnumOption('tools', 'Toolset to compile with, default = platform default (msvc under windows)', 'mingw', ['mingw', 'default']),
	EnumOption('mode', 'Compile mode', 'debug', ['debug', 'release']),
	BoolOption('nativestl', 'Try to use native STL instead of STLPort', 'no')
)

gcc_flags = {
	'common': ['-g', '-Wall', '-Wextra', '-Wno-unused-parameter', '-pipe', '-fexceptions'],
	'debug': [], 
	'release' : ['-O2']
}

msvc_flags = {
	# 4512: assn not generated, 4100: <something annoying, forget which>, 4189: var init'd, unused, 4996: fn unsafe, use fn_s
	# 4121: alignment of member sensitive to packing
	'common' : ['/W4', '/EHsc', '/Zi', '/GR', '/wd4121', '/wd4100', '/wd4189', '/wd4996', '/wd4512'],
	'debug' : ['/MT'],
	'release' : ['/O2', '/MT']
}

gcc_link_flags = {
	'common' : ['-g', '-Wl,--no-undefined'],
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

mode = env['mode']
if mode not in gcc_flags:
	print "Unknown mode, exiting"
	Exit(1)

dev = Dev(mode, tools, env)
dev.prepare()

env.SConsignFile()
env.Tool("gch", toolpath=".")

env.Append(CPPPATH = ["#/boost/boost/tr1/tr1/", "#/boost/"])

if not env['nativestl']:
	env.Append(CPPPATH = ['#/stlport/stlport/'])
	env.Append(LIBPATH = ['#/stlport/lib/'])
	env.Append(CPPDEFINES = ['HAVE_STLPORT', '_STLP_USE_STATIC_LIB=1'])
	if mode == 'debug':
		env.Append(LIBS = ['stlportg.5.1'])
	else:
		env.Append(LIBS = ['stlport.5.1'])	
	

if 'mingw' not in env['TOOLS'] and 'gcc' in env['TOOLS']:
	print "Non-mingw gcc builds not supported"
	Exit(1)

if env['CC'] == 'cl':
	flags = msvc_flags
	link_flags = msvc_link_flags
	defs = msvc_defs
	
	env.Append(LIBS = ['User32', 'shell32', 'Advapi32'])
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

from makedefs import convert
env.Command('client/StringDefs.cpp', 'client/StringDefs.h', lambda target, source, env: convert())
env.Depends('client/StringDefs.cpp', 'client/StringDefs.h')
env.SideEffect('Example.xml', 'client/StringDefs.cpp')

dev.zlib = dev.build('zlib/')
dev.bzip2 = dev.build('bzip2/')
dev.boost = dev.build('boost/')
dev.yassl = dev.build('yassl/')
dev.client = dev.build('client/')
env.Depends(dev.client, 'client/StringDefs.cpp')
dev.smartwin = dev.build('smartwin/')
dev.win32 = dev.build('win32/')
