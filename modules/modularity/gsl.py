#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty gsl detection
"""

import platform, os
from waflib.Configure import conf
from waflib import Utils, Logs

#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

GSL_HEADER_FILES = ['gsl/gsl_math.h', 'gsl/gsl_eigen.h', 'gsl/gsl_matrix.h', 'gsl/gsl_blas.h']
GSL_LIBRARY_FILES = ['gsl','gslcblas']
GSL_LIBRARY_EXTENSIONS = ['.a', '.so']

@conf
def check_gsl(conf):
        env = conf.env
        opt = conf.options
        env['GSL_FOUND'] = False
        
        # Gather library and include paths to check
        includes_check = []
        libs_check = []
        if conf.options.gsl:
                includes_check = [opt.gsl + '/include']
                libs_check = [opt.gsl + '/lib']
        else:
                if 'CPPFLAGS' in os.environ:
                        includes_check += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
                includes_check += ['/usr/local/include', '/usr/include']
                libs_check= ['/usr/lib', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu', '/usr/local/lib/x86_64-linux-gnu']
                if 'LD_LIBRARY_PATH' in os.environ:
                        libs_check += os.environ['LD_LIBRARY_PATH'].split(":")
        
        # Search for header files in the include paths
        include_paths = set()
        try:
                conf.start_msg('Checking for GSL include files')
                if Logs.verbose: 
                    Logs.pprint('CYAN', '...')
                    Logs.pprint('CYAN', '  searching in paths: %s' % includes_check)
                for header_file in GSL_HEADER_FILES:
                    if Logs.verbose: Logs.pprint('CYAN', '    checking header file : %s' % header_file)
                    res = conf.find_file(header_file, includes_check)
                    i = includes_check.index(res[:-len(header_file)-1])
                    include_paths.add(includes_check[i])
                conf.env.INCLUDES_GSL = list(include_paths)
        except:
                conf.end_msg('Not found', 'RED')
                return 0
        else:
                conf.end_msg('ok')
                if Logs.verbose: Logs.pprint('CYAN', '   path : %s' % include_paths)

        # Search for library files in the library paths
        lib_paths = set()
        try:
                conf.start_msg('Checking for GSL libs')
                if Logs.verbose:
                    Logs.pprint('CYAN', '...')
                    Logs.pprint('CYAN', '  searching in paths: %s' % libs_check)
                for l in GSL_LIBRARY_FILES:
                        found = False
                        for ext in GSL_LIBRARY_EXTENSIONS:
                                lib_full_name = 'lib' + l + ext
                                if Logs.verbose:
                                    Logs.pprint('CYAN', '     looking for file : %s' % lib_full_name)
                                try:
                                    res = conf.find_file(lib_full_name, libs_check)
                                    found = True
                                    break
                                except:
                                    pass
                        if not found: raise Exception("Library not found.")
                        index = libs_check.index(res[:-len(lib_full_name)-1])
                        lib_paths.add(libs_check[index])
                conf.env.LIBPATH_GSL = lib_paths
                conf.env.LIB_GSL = GSL_LIBRARY_FILES
                conf.env['GSL_FOUND'] = True
        except:
                conf.end_msg('Not found', 'RED')
                return 0
        else:
                conf.end_msg('ok')
                if Logs.verbose:
                        Logs.pprint('CYAN', '   paths : %s' % lib_paths)
                        Logs.pprint('CYAN', '   libs : %s' % GSL_LIBRARY_FILES)
        return 1
       


def detect(conf):
        return check_gsl(conf)

def options(opt):
        opt.add_option('--gsl', type='string', help='path to GSL', dest='gsl')

