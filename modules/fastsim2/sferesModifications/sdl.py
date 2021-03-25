#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty sdl detection
"""

import os, glob, types
import Options, Configure


def detect_sdl(conf):
	env = conf.env
	opt = Options.options

	conf.env['LIB_SDL'] = ''
	conf.env['LIB_SDL_gfx'] = ''
	conf.env['SDL_FOUND'] = False
	libPath = '-Wl,-rpath,'
	if Options.options.sdl:
		conf.env['CPPPATH_SDL'] = Options.options.sdl + '/include/SDL'
		conf.env['CPPPATH_SDL_gfx'] = Options.options.sdl + '/include/SDL'
		conf.env['LIBPATH_SDL'] = Options.options.sdl + '/lib'
		conf.env['LIBPATH_SDL_gfx'] = Options.options.sdl + '/lib'
		libPath += (Options.options.sdl + '/lib')
	else:
		conf.env['CPPPATH_SDL'] = ['/usr/include/SDL', '/usr/local/include/SDL', '/usr/include' ]
		conf.env['CPPPATH_SDL_gfx'] = ['/usr/include/SDL', '/usr/local/include/SDL', '/usr/include']
		conf.env['LIBPATH_SDL'] = ['/usr/lib', '/usr/local/lib', '/usr/lib/SDL']
		conf.env['LIBPATH_SDL_gfx'] = ['/usr/lib', '/usr/local/lib', '/usr/lib/SDL']
		libPath += '/usr/local/lib'
	res = Configure.find_file('SDL.h', conf.env['CPPPATH_SDL_gfx'] )
	conf.check_message('header','SDL.h', (res != '') , res)
	if (res == '') :
		return 0
	conf.env['CCDEFINES_SDL']=['_GNU_SOURCE=1', '_REENTRANT']
	conf.env['CCDEFINES_SDL_gfx']=['_GNU_SOURCE=1', '_REENTRANT']
	conf.env['CXXDEFINES_SDL']=['_GNU_SOURCE=1', '_REENTRANT']
	conf.env['CXXDEFINES_SDL_gfx']=['_GNU_SOURCE=1', '_REENTRANT']
	conf.env['SDL_FOUND'] = True
	conf.env['HAVE_SDL'] = 1
	conf.env['HAVE_SDL_gfx'] = 1
	conf.env['LIB_SDL'] = ['SDL', 'pthread']
	conf.env['LIB_SDL_gfx'] = ['SDL_gfx']
	conf.env['LINKFLAGS_SDL'] = [libPath]
	
	return 1

def detect(conf):
	return detect_sdl(conf)

def set_options(opt):
	opt.add_option('--sdl', type='string', help='path to SDL', dest='sdl')
