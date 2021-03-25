#! /usr/bin/env python
# encoding: utf-8
# Joost Huizinga - 2016

"""
Quick n dirty open gl detection
"""
import platform
import os
from waflib import Logs # Allows writing in response to the --verbose option
from waflib.Configure import conf # The decorator
from waflib import Utils, Logs

MAC_OSX_OLD_HEADER_DIR = '/System/Library/Frameworks/OpenGL.framework/Headers'
MAC_OSX_OLD_LIB_DIR = '/System/Library/Frameworks/OpenGL.framework/Libraries'
MAC_OSX_XCODE5_HEADER_DIR = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform' + \
                            '/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Headers'
MAC_OSX_XCODE5_LIB_DIR = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform' + \
                            '/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Libraries'


def options(opt):
	opt.add_option('--opengl', type='string', help='location of the opengl', dest='opengl')
	opt.add_option('--glut', type='string', help='location of the glut', dest='glut')
	opt.add_option('--glut-include', type='string', help='location of the glut header files', dest='glut_include')
	opt.add_option('--glut-lib', type='string', help='location of the glut library files', dest='glut_lib')

def __check_bullet_osx(conf):
        # If this is an older version of mac osx, the header files (and library files) should be here
        if os.path.isdir(MAC_OSX_OLD_HEADER_DIR):
                conf.env.INCLUDES_OPENGL = [MAC_OSX_OLD_HEADER_DIR]
                conf.env.LIBPATH_OPENGL = [MAC_OSX_OLD_LIB_DIR]
        else:
                conf.env.INCLUDES_OPENGL = [MAC_OSX_XCODE5_HEADER_DIR]
                conf.env.LIBPATH_OPENGL = [MAC_OSX_XCODE5_LIB_DIR]
        
        
def __bullet_header_found(conf, mac_framework=True):
	conf.end_msg('ok')
	conf.env['OPENGL_FOUND'] = True
	if mac_framework:
		conf.env.FRAMEWORK_OPENGL = ['OpenGL', 'Cocoa']
	else:
		conf.env.LIB_OPENGL = ['GL','GLU']
        	if Logs.verbose:
                	Logs.pprint('CYAN', '   libs : %s' % conf.env.LIB_OPENGL )


@conf
def check_opengl(conf):
        conf.env['OPENGL_FOUND'] = False
        mac_framework = False
        if conf.options.opengl:
                conf.env.INCLUDES_OPENGL = [conf.options.opengl + '/include']
                conf.env.LIBPATH_OPENGL = [conf.options.opengl + '/lib']
        elif platform.system() == 'Darwin':
                __check_bullet_osx(conf)
                mac_framework = True
        else:
                conf.env.INCLUDES_OPENGL = ['/usr/include', '/usr/local/include', '/usr/local/include/GL', '/usr/include/GL']
                conf.env.LIBPATH_OPENGL = ['/usr/lib', '/usr/local/lib']

        conf.start_msg('Checking for OpenGL include')
        if Logs.verbose: Logs.pprint('CYAN', '...')
        if Logs.verbose: Logs.pprint('CYAN', '  Searching in: %s' % conf.env.INCLUDES_OPENGL)
        # The location and name of the open gl header file in the old version of osx
        try:
                if Logs.verbose: Logs.pprint('CYAN', '    Searching for OpenGL/gl.h')    
                res = conf.find_file('OpenGL/gl.h', conf.env.INCLUDES_OPENGL)
                if Logs.verbose: Logs.pprint('CYAN', '    Found at: %s' % res)
                __bullet_header_found(conf, mac_framework)
                return 1
        except:
                if Logs.verbose: Logs.pprint('CYAN', '    Not found')
                pass

        # The location an name of the open gl header file in more recent versions of osx
        try:
                if Logs.verbose: Logs.pprint('CYAN', '    Searching for gl.h')
                res = conf.find_file('gl.h', conf.env.INCLUDES_OPENGL)
                if Logs.verbose: Logs.pprint('CYAN', '    Found at: %s' % res)
                __bullet_header_found(conf, mac_framework)
                return 1
        except:
                if Logs.verbose: Logs.pprint('CYAN', '    Not found')
                pass

        # The location an name of the open gl header file on linux (I think)
        # The location an name of the open gl header file on linux (I think)
        try:
                if Logs.verbose: Logs.pprint('CYAN', '    Searching for OpenGL.h')
                res = conf.find_file('OpenGl.h', conf.env.INCLUDES_OPENGL)
                if Logs.verbose: Logs.pprint('CYAN', '    Found at: %s' % res)
                __bullet_header_found(conf, mac_framework)
                return 1
        except:
                if Logs.verbose: Logs.pprint('CYAN', '    Not found')
                pass
        # The location of the open gl header file on titan
        try:
                if Logs.verbose: Logs.pprint('CYAN', '    Searching for GL/gl.h')
                res = conf.find_file('GL/gl.h', conf.env.INCLUDES_OPENGL)
                if Logs.verbose: Logs.pprint('CYAN', '    Found at: %s' % res)
                __bullet_header_found(conf, mac_framework)
                return 1
        except:
                conf.end_msg('Not found', 'RED')
        return 1

@conf
def check_glut(conf):
	conf.env['GLUT_FOUND'] = False
	conf.env.INCLUDES_GLUT = []
	conf.env.LIBPATH_GLUT = []
	if conf.options.glut_include:
		conf.env.INCLUDES_GLUT += [conf.options.glut_include, conf.options.glut_include + '/GL']
	if conf.options.glut_lib:
		conf.env.LIBPATH_GLUT += [conf.options.glut_lib]
	if conf.options.glut:
		conf.env.INCLUDES_GLUT += [conf.options.glut + '/include', conf.options.glut + '/include/GL']
		conf.env.LIBPATH_GLUT += [conf.options.glut + '/lib']
	if len(conf.env.INCLUDES_GLUT) == 0 or len(conf.env.LIBPATH_GLUT) == 0:
		conf.env.INCLUDES_GLUT += ['/usr/include', '/usr/local/include', '/usr/include/GL', '/usr/local/include/GL']
		conf.env.LIBPATH_GLUT += ['/usr/lib', '/usr/local/lib']

	conf.start_msg('Checking for GLUT include')

	if Logs.verbose:
		Logs.pprint('CYAN', '\n   looking in paths : %s' % conf.env.INCLUDES_GLUT )
	# The location and name of the open gl header file in the old version of osx
	try:
		conf.find_file('glut.h', conf.env.INCLUDES_GLUT)
		conf.end_msg('ok')
		conf.env['GLUT_FOUND'] = True
		conf.env.LIB_GLUT = ['glut']
	except:
		conf.end_msg('Not found', 'RED')
        if Logs.verbose:
                Logs.pprint('CYAN', '   looking in library paths : %s' % conf.env.LIBPATH_GLUT )
	return 1

def detect(conf):
        check_opengl(conf)
        check_glut(conf)
        return 1
