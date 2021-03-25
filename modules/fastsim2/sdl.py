#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty sdl detection
"""

import platform, os
from waflib.Configure import conf
from waflib import Utils, Logs

SDL_HEADER_FILE = 'SDL.h'
SDL_LIBRARY_FILES = ['SDL', 'SDLmain']
SDL_LIBRARY_EXTENSIONS = ['.a', '.so']

def __find_file_in_list(conf, filename, pathlist):
	for path in pathlist:
		try:
			conf.find_file(filename, path)
			return path
		except:
			pass
	return pathlist

@conf
def check_sdl(conf):
	env = conf.env
	opt = conf.options
	env['SDL_FOUND'] = False
	#libPath = '-Wl,-rpath,'
	includes_check = []
	libs_check = []
	if conf.options.sdl:
		includes_check = [opt.sdl + '/include/SDL']
		libs_check = [opt.sdl + '/lib']
		#libPath += (opt.sdl + '/lib')
	else:
		if 'CPPFLAGS' in os.environ:
			includes_check += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
		includes_check += ['/usr/include/SDL', '/usr/local/include/SDL', '/usr/include']
		libs_check= ['/usr/lib', '/usr/local/lib', '/usr/lib/SDL']
		if 'LD_LIBRARY_PATH' in os.environ:
			libs_check += os.environ['LD_LIBRARY_PATH'].split(":")
		#libPath += '/usr/local/lib'
		
	try:
		conf.start_msg('Checking for SDL include (optional)')
		#env.INCLUDES_SDL =  __find_file_in_list(conf, SDL_HEADER_FILE, env.INCLUDES_SDL)
		res = conf.find_file(SDL_HEADER_FILE, includes_check)
		i_index = includes_check.index(res[:-len(SDL_HEADER_FILE)-1])
	except:
		conf.end_msg('Not found', 'YELLOW')
		return 0
	else:
		conf.end_msg('ok')
		if Logs.verbose:
			Logs.pprint('CYAN', '	path : %s' % includes_check[i_index])

	try:
		conf.start_msg('Checking for SDL libs (optional)')
		lib_paths = []
		libs  = SDL_LIBRARY_FILES
		for l in libs:
			found = False
			lib_full_name = ''
			for ext in SDL_LIBRARY_EXTENSIONS:
				lib_full_name = 'lib' + l + ext
				try:
					res = conf.find_file(lib_full_name, libs_check)
					found = True
					break
				except:
					pass
			if not found: raise Exception("Library not found.")
			index = libs_check.index(res[:-len(lib_full_name)-1])
			if libs_check[index] not in lib_paths:
				lib_paths += [libs_check[index]]
		conf.env.INCLUDES_SDL = includes_check[i_index]
		conf.env.LIBPATH_SDL = lib_paths
		conf.env.LIB_SDL = libs
		#env.CCDEFINES_SDL = ['_GNU_SOURCE=1', '_REENTRANT', 'USE_SDL']
		#env.CXXDEFINES_SDL = ['_GNU_SOURCE=1', '_REENTRANT', 'USE_SDL']
		conf.env.DEFINES_SDL = ['USE_SDL']
		conf.env['SDL_FOUND'] = True
		conf.env['HAVE_SDL'] = 1
		if platform.system() == 'Darwin':
			env.FRAMEWORK_SDL += ['Cocoa']
		#env.LINKFLAGS_SDL = [libPath]
	except:
		conf.end_msg('Not found', 'YELLOW')
		return 0
	else:
		conf.end_msg('ok')
		if Logs.verbose:
			Logs.pprint('CYAN', '	paths : %s' % lib_paths)
			Logs.pprint('CYAN', '	libs : %s' % libs)
	return 1
# 	conf.start_msg('Checking for SDL libraries file (optional)')
# 	lib_paths = []
# 	libs  = SDL_LIBRARY_FILES
# 	for l in libs:
# 		res = conf.find_file(l, libs_check)
# 		index = libs_check.index(res[:-len(l)-1])
# 		if libs_check[index] not in lib_paths:
# 			lib_paths += [libs_check[index]]
# 	try:
# 		res = conf.find_file(SDL_LIBRARY_FILE, env.LIBPATH_SDL)
# 	except:
# 		conf.end_msg('Not found', 'YELLOW')
# 		return 0
# 
# 
# 	conf.end_msg('ok')
# 	#env.CCDEFINES_SDL = ['_GNU_SOURCE=1', '_REENTRANT', 'USE_SDL']
# 	#env.CXXDEFINES_SDL = ['_GNU_SOURCE=1', '_REENTRANT', 'USE_SDL']
# 	env.DEFINES_SDL = ['USE_SDL']
# 	env['SDL_FOUND'] = True
# 	env['HAVE_SDL'] = 1
# 	env.LIB_SDL = ['SDL', 'pthread', 'SDLmain']
# 	env.LINKFLAGS_SDL = [libPath]
# 	if platform.system() == 'Darwin':
# 		env.FRAMEWORK_SDL += ['Cocoa']
	


def detect(conf):
	return check_sdl(conf)

def options(opt):
	opt.add_option('--sdl', type='string', help='path to SDL', dest='sdl')
