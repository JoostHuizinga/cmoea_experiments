#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2014

"""
Quick n dirty tbb detection
"""

import os
from waflib.Configure import conf
from waflib import Utils, Logs
import traceback

TBB_LIB_EXT = ['.so', '.a', '.dylib']

def options(opt):
    opt.add_option('--tbb', type='string', help='path to Intel TBB', dest='tbb')


@conf
def check_tbb(self, *k, **kw):
    if self.options.tbb:
        includes_tbb = [self.options.tbb + '/include']
        libpath_tbb = [self.options.tbb + '/lib']
    else:
        includes_tbb = ['/usr/local/include', '/usr/include', '/opt/intel/tbb/include']
        libpath_tbb = ['/usr/local/lib', '/usr/lib', '/opt/intel/tbb/lib']
        if 'CPPFLAGS' in os.environ:
            includes_tbb += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
        if 'LD_LIBRARY_PATH' in os.environ:
            libpath_tbb += os.environ['LD_LIBRARY_PATH'].split(":")

    self.start_msg('Checking Intel TBB includes (optional)')
    try:
        res = self.find_file('tbb/parallel_for.h', includes_tbb)
        index = includes_tbb.index(res[:-len('tbb/parallel_for.h')-1])
        includes_tbb = [includes_tbb[index]]
        self.end_msg('ok')
        if Logs.verbose:
            Logs.pprint('CYAN', '	path : %s' % includes_tbb[0])
    except:
        self.end_msg('not found', 'YELLOW')
        return

    self.start_msg('Checking Intel TBB libs (optional)')
    if Logs.verbose: Logs.pprint('CYAN', '...')

    # Define these here in case TBB_LIB_EXT is empty
    paths_found = []
    found = False
    for ext in TBB_LIB_EXT:
        paths_found = []
        found = False
        lib = 'lib' + 'tbb' + ext
        if Logs.verbose: Logs.pprint('CYAN', '   Looking for file: %s ' % lib)
        try:
            res = self.find_file(lib, libpath_tbb)
            if Logs.verbose: Logs.pprint('CYAN', '    found')
            found = True
            index = libpath_tbb.index(res[:-len(lib)-1])
            if libpath_tbb[index] not in paths_found:
                paths_found.append(libpath_tbb[index])
            break
        except:
            #traceback.print_exc()
            if Logs.verbose: Logs.pprint('CYAN', '    not found')
            continue
    if not found:
        self.end_msg('not found', 'YELLOW')
        return 1
    self.end_msg('ok')

    if Logs.verbose:
        Logs.pprint('CYAN', '	path : %s' % paths_found)
        Logs.pprint('CYAN', '	libs : [\'tbb\']')

    self.env.LIBPATH_TBB = paths_found
    self.env.LIB_TBB = ['tbb']
    self.env.INCLUDES_TBB = includes_tbb
    self.env['TBB_ENABLED'] = True
