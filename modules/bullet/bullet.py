#! /usr/bin/env python
# encoding: utf-8
# Joost Huizinga - 2016

"""
Quick n dirty bullet detection
"""
from waflib.Configure import conf # The @conf decorator
from waflib import Logs # Allows writing in response to the --verbose option
import traceback # For debugging
import os # For checking environment variables

BULLET_HEADER_FILES = ['btBulletCollisionCommon.h', 'btBulletDynamicsCommon.h', 'LinearMath/btAlignedObjectArray.h']
BULLET_LIBRARY_FILES = ['BulletDynamics', 'BulletCollision', 'LinearMath']
BULLET_LIBRARY_EXTENSIONS = ['.a', '.so']

def options(opt):
        opt.add_option('--bullet', type='string', help='location of the bullet installation', dest='bullet')


@conf
def check_bullet(conf):
    conf.env['BULLET_FOUND'] = False

    # Gathering include and library paths to check
    include_check = []
    lib_check = []
    if conf.options.bullet:
            include_check = [
                    conf.options.bullet + '/src', 
                    conf.options.bullet + '/Demos', 
                    conf.options.bullet + '/include', 
                    conf.options.bullet + '/include/bullet'
                    ]
            lib_check = [
                    conf.options.bullet + '/lib', 
                    conf.options.bullet + '/src/.libs',
                    ]
    else:
        if 'CPPFLAGS' in os.environ:
            include_check += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
        include_check += ['/usr/include', '/usr/include/bullet', '/usr/local/include', '/usr/local/include/bullet']
        if 'LD_LIBRARY_PATH' in os.environ:
            lib_check += os.environ['LD_LIBRARY_PATH'].split(":")
        lib_check += ['/usr/lib', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu', '/usr/local/lib/x86_64-linux-gnu']

    # Searching for header files in provided include directories
    files_found = set()
    include_directories = set()
    conf.start_msg('Checking for Bullet include files')
    try:
            if Logs.verbose: Logs.pprint('CYAN', '...')
            for include_dir in include_check:
                    if Logs.verbose: Logs.pprint('CYAN', '   checking path : %s' % include_dir)
                    try:
                            for header_file in BULLET_HEADER_FILES:
                                    if header_file not in files_found:
                                        conf.find_file(header_file, include_dir)
                                        files_found.add(header_file)
                                        if os.path.split(include_dir)[1] == "bullet":
                                            include_directories.add(os.path.split(include_dir)[0])
                                        include_directories.add(include_dir)
                            break
                    except:
                            pass
            if len(files_found) != len(BULLET_HEADER_FILES): raise Exception("Header file not found.")
            conf.env.INCLUDES_BULLET = list(include_directories)
            conf.end_msg('ok')
            if Logs.verbose:
                    Logs.pprint('CYAN', '   path : %s' % conf.env.INCLUDES_BULLET)
    except:
            not_found = [ file_name for file_name in BULLET_HEADER_FILES if file_name not in files_found ]
            conf.end_msg('Not found: ' + str(not_found), 'RED')
            return 0

    # Searching for library files in provided library directories
    files_found = set()
    conf.start_msg('Checking for Bullet libraries')
    try:
            if Logs.verbose: Logs.pprint('CYAN', '...')
            found = False
            full_name_found = []
            for lib_dir in lib_check:
                    if Logs.verbose: Logs.pprint('CYAN', '  checking path : %s' % lib_dir)
                    for ext in BULLET_LIBRARY_EXTENSIONS:
                            try:
                                    full_name_found = []
                                    for lib_file in BULLET_LIBRARY_FILES:
                                            lib_full_name = 'lib' + lib_file + ext
                                            if Logs.verbose:
                                                    Logs.pprint('CYAN', '     looking for file : %s' % lib_full_name)
                                            conf.find_file(lib_full_name, lib_dir)
                                            files_found.add(lib_file)
                                            full_name_found.append(lib_full_name)
                                    conf.env.LIBPATH_BULLET = lib_dir
                                    found = True
                                    break
                            except:
                                    pass
            if not found: raise Exception("Library not found.")
            conf.env['BULLET_FOUND'] = True
            conf.env.LIB_BULLET = BULLET_LIBRARY_FILES 
            conf.end_msg('ok')
            if Logs.verbose:
                    Logs.pprint('CYAN', '   paths : %s' % conf.env.LIBPATH_BULLET)
                    Logs.pprint('CYAN', '   libs : %s' % full_name_found)
    except:
            not_found = [ file_name for file_name in BULLET_LIBRARY_FILES if file_name not in files_found ]
            conf.end_msg('Not found'  + str(not_found), 'RED')
            traceback.print_exc()
            return 0
    if conf.options.bullet_double_precision:
        conf.env.DEFINES_BULLET = ['BT_USE_DOUBLE_PRECISION']
    return 1
    

def detect(conf):
        return check_bullet(conf)
