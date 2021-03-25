#! /usr/bin/env python
# encoding: utf-8
# Joost Huizinga - 2016

"""
Quick n dirty bullet detection
"""
from waflib.Configure import conf # The @conf decorator
from waflib import Logs # Allows writing in response to the --verbose option
import subprocess

FREETYPE_HEADER_FILE='ft2build.h'

def options(opt):
	opt.add_option('--freetype', type='string', help='location of freetype2', dest='freetype')

def __find_file_in_list(conf, filename, pathlist):
	for path in pathlist:
		try:
			conf.find_file(filename, path)
			return [path]
		except:
			pass
	return pathlist

@conf
def check_freetype(conf):
	conf.env['FREETYPE_FOUND'] = False
	if conf.options.freetype:
		conf.env.INCLUDES_FREETYPE = [conf.options.freetype + '/include', conf.options.freetype + '/include/freetype2']
		conf.env.LIBPATH_FREETYPE = [conf.options.freetype + '/lib']
	else:
		conf.env.INCLUDES_FREETYPE = ['/usr/include', '/usr/local/include', '/usr/include/freetype2', '/usr/local/include/freetype2']
		conf.env.LIBPATH_FREETYPE = ['/usr/lib', '/usr/local/lib']
	try:
		conf.start_msg('Checking for Freetype2 include')
		conf.env.INCLUDES_FREETYPE =  __find_file_in_list(conf, FREETYPE_HEADER_FILE, conf.env.INCLUDES_FREETYPE)
		conf.find_file('ft2build.h', conf.env.INCLUDES_FREETYPE)

                # Freetype-config may provide additional paths that need to be added for freetype to work properly.
                # For this, we trust freetype-config, we will not check whether certain files are actually in this path.
                output = subprocess.check_output(["freetype-config", "--cflags"])
                additional_includes = [path[2:] for path in output.split() if path[0:2] == '-I']
                for include in additional_includes:
                        if include not in conf.env.INCLUDES_FREETYPE:
                            conf.env.INCLUDES_FREETYPE.append(include)

		conf.end_msg('ok')
                if Logs.verbose: Logs.pprint('CYAN', '    Paths: %s' % conf.env.INCLUDES_FREETYPE)
		conf.env['FREETYPE_FOUND'] = True
		conf.env.LIB_FREETYPE = ['freetype']
	except:
		conf.end_msg('Not found', 'RED')
	return 1

def detect(conf):
	return check_bullet(conf)
