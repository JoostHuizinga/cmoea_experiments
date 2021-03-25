#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty mpi detection
"""

import os, glob, types
from waflib.Configure import conf
from waflib import Utils, Logs
import traceback


def options(opt):
	opt.add_option("--no-mpi",
		       default=False, action='store_true',
		       help='disable mpi', dest='no_mpi')
	opt.add_option('--mpi', type='string', help='path to mpi', dest='mpi')
	opt.add_option('--force-mpi', default=False, action='store_true', help='path to mpi', dest='force_mpi')


MPI_LIB_NAME = 'mpi'
MPI_LIBS = [['mpi'], ['mpi_cxx', 'mpicxx', '']]
MPI_LIB_EXTENSIONS = ['.a', '.so', '.dylib']

@conf
def check_mpi(conf):
	opt = conf.options

	conf.env['LIB_MPI'] = ''
	conf.env['MPI_FOUND'] = False
	includes_check = []
	libs_check = []
	if conf.options.no_mpi :
		return
	elif conf.options.force_mpi :
		conf.start_msg('Checking for MPI include (optional)')
		conf.end_msg('ok')
		if Logs.verbose:
			Logs.pprint('CYAN', '  MPI forced')
		conf.start_msg('Checking for MPI libs (optional)')
		conf.end_msg('ok')
		if Logs.verbose:
			Logs.pprint('CYAN', '  MPI forced')
                conf.env.INCLUDES_MPI = [conf.options.mpi + '/include']
                conf.env.LIBPATH_MPI = [conf.options.mpi + '/lib']
                conf.env['MPI_FOUND'] = True
                conf.env.LIB_MPI = []
		return 1
		
	if conf.options.mpi:
		includes_check = [conf.options.mpi + '/include']
		libs_check = [conf.options.mpi + '/lib']
	else:
		includes_check = ['/usr/include/mpi', '/usr/local/include/mpi', '/usr/include', '/usr/local/include']
		libs_check = ['/usr/lib', '/usr/local/lib', '/usr/lib/openmpi']
		if 'MPI_ROOT' in os.environ:
			includes_check += [os.environ['MPI_ROOT'] +"/include", os.environ['MPI_ROOT'] +"/include64"]
		elif 'I_MPI_ROOT' in os.environ:
			includes_check += [os.environ['I_MPI_ROOT'] +"/include", os.environ['I_MPI_ROOT'] +"/include64"]
		elif 'CPPFLAGS' in os.environ:
			includes_check += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
		if 'LD_LIBRARY_PATH' in os.environ:
			libs_check += os.environ['LD_LIBRARY_PATH'].split(":")

	try:
		conf.start_msg('Checking for MPI include (optional)')
		res = conf.find_file('mpi.h', includes_check)
		i_index = includes_check.index(res[:-len('mpi.h')-1])
		conf.end_msg('ok')

		if Logs.verbose:
			Logs.pprint('CYAN', '	path : %s' % includes_check[i_index])

		conf.start_msg('Checking for MPI libs (optional)')
		if Logs.verbose: Logs.pprint('CYAN', '...')

		# We put the extensions in the outer loop, because we want all found libraries to have the same extension
		lib_found = False
		lib_paths = []
		libs_found = []
		for ext in MPI_LIB_EXTENSIONS:
			# Reset the library paths
			lib_paths = []
			libs_found = []
			for alt_libs in MPI_LIBS:

				# We should find one of the libraries in alt_libs
				lib_found = False
				for alt_lib in alt_libs:

					# The empty string indicates that this library is optional
					if alt_lib == '':
						lib_found = True
						if Logs.verbose: Logs.pprint('CYAN', '    No library found for: %s. Assuming that compiler knows best.' % alt_libs)
						break

					lib = 'lib' + alt_lib + ext
					if Logs.verbose: Logs.pprint('CYAN', '    Looking for library: %s' % lib)
					try:
						res = conf.find_file(lib, libs_check)
						if Logs.verbose: Logs.pprint('CYAN', '    found')
						lib_found = True
						libs_found.append(alt_lib)
						index = libs_check.index(res[:-len(lib)-1])
						if libs_check[index] not in lib_paths:
							lib_paths += [libs_check[index]]
						break
					except:
						#traceback.print_exc()
						if Logs.verbose: Logs.pprint('CYAN', '    not found')
						continue

				# If we did not find one the libraries, try the next extension
				if not lib_found:
					break

			# If we found all libraries, our job is done
			if lib_found:
				break

		if not lib_found:
			raise

#		lib_paths = []
#		libs  = ['lib' + MPI_LIB_NAME + '.so']
#		for l in libs:
#		    res = conf.find_file(l, libs_check)
#		    index = libs_check.index(res[:-len(l)-1])
#			if libs_check[index] not in lib_paths:
#				lib_paths += [libs_check[index]]
#
#		if Logs.verbose:
#			Logs.pprint('CYAN', '   found lib : %s' % libs)
#
#		anta_libs = ['mpi_cxx', 'mpicxx']
#		found = False
#		ii = -1
#		for i in range(len(anta_libs)):
#			l = 'lib'+anta_libs[i]+'.so'
#			try:
#				res = conf.find_file(l, libs_check)
#				index = libs_check.index(res[:-len(l)-1])
#				if libs_check[index] not in lib_paths:
#					lib_paths += [libs_check[index]]
#				found = True
#				ii = i
#				break
#			except:
#				continue
#		if not found:
#			conf.end_msg('Not found', 'RED')
#			return 1
		conf.end_msg('ok')

		if Logs.verbose:
			Logs.pprint('CYAN', '	paths : %s' % lib_paths)
			Logs.pprint('CYAN', '	libs : %s' % libs_found)

		conf.env.INCLUDES_MPI = includes_check[i_index]
		conf.env.LIBPATH_MPI = lib_paths
		conf.env['MPI_FOUND'] = True
		conf.env.LIB_MPI = libs_found
	except:
		conf.end_msg('Not found', 'RED')
	return 1

def detect(conf):
	return detect_mpi(conf)
