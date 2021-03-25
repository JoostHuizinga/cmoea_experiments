#! /usr/bin/env python
# encoding: utf-8
# Joost Huizinga - 2017

"""
Quick n dirty pngwriter detection
"""
from waflib.Configure import conf # The @conf decorator
from waflib import Logs # Allows writing in response to the --verbose option
import subprocess
import traceback

PNGWRITER_HEADER_FILE='pngwriter.h'

def options(opt):
	opt.add_option('--pngwriter', type='string', help='location of pngwriter', dest='pngwriter')

def __find_file_in_list(conf, filename, pathlist):
	for path in pathlist:
		try:
                        if Logs.verbose: 
                            Logs.pprint('CYAN', '    Looking for %s in %s' % filename, path)
			conf.find_file(filename, path)
			return [path]
		except:
			pass
	return pathlist

@conf
def check_pngwriter(conf):
	conf.env['PNGWRITER_FOUND'] = False
	if conf.options.pngwriter:
		conf.env.INCLUDES_PNGWRITER = [conf.options.pngwriter + '/include']
		conf.env.LIBPATH_PNGWRITER = [conf.options.pngwriter + '/lib']
	else:
		conf.env.INCLUDES_PNGWRITER = ['/usr/include', '/usr/local/include']
		conf.env.LIBPATH_PNGWRITER = ['/usr/lib', '/usr/local/lib']
	try:
		conf.start_msg('Checking for pngwriter include')
		conf.env.INCLUDES_PNGWRITER =  __find_file_in_list(conf, PNGWRITER_HEADER_FILE,
                        conf.env.INCLUDES_PNGWRITER)
		conf.find_file(PNGWRITER_HEADER_FILE, conf.env.INCLUDES_PNGWRITER)
		conf.end_msg('ok')
                if Logs.verbose: Logs.pprint('CYAN', '    Paths: %s' % conf.env.INCLUDES_PNGWRITER)
		conf.env['PNGWRITER_FOUND'] = True
		conf.env.LIB_PNGWRITER = ['pngwriter', 'png']
	except:
		conf.end_msg('Not found', 'RED')
                if Logs.verbose: traceback.print_exc()
	return 1

def detect(conf):
	return check_bullet(conf)
