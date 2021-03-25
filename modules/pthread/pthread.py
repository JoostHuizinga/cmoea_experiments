#! /usr/bin/env python
# encoding: utf-8

"""
Add the pthread compiler and linker flags for files that specify uselib=PTHREAD
"""

import os
from waflib.Configure import conf
from waflib import Utils, Logs
import traceback


def options(opt):
    pass


@conf
def check_pthread(self, *k, **kw):
    self.env.CXXFLAGS_PTHREAD = ['-pthread']
    self.env.LINKFLAGS_PTHREAD = ['-pthread']
    self.env['PTHREAD'] = True
