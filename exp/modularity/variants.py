#!/usr/bin/env python
from waflib.Configure import conf
import sferes

def build(bld):
    uselib_ = 'TBB BOOST EIGEN PTHREAD GSL MPI BULLET OGLFT FREETYPE OPENGL GLUT PNGWRITER'
    cxxflags_kdtree = bld.get_env()['CXXFLAGS', '-I.', '-DLIBSSRCKDTREE_HAVE_BOOST']

    lzw = bld.stlib(features = 'cxx cxxstlib',
                    source = 'lzw.cpp',
                    includes = '. ../../',
                    target = 'lzw',
                    uselib = uselib_)
    
    # Experiment for testing MPCMOEA on a small problem
    if(False):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['DBG MRV1 MRS6 POP2 GEN10 BIN4 NOGL'])
    
    # Experiment for testing MPCMOEA release version on the six-tasks problem
    if(False):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['RLS MRV1 MRS5 POP2 GEN10 BIN4 NOGL'])
    
    # Debug settings for a neural network with multiple layers
    if(False):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['DBG MRV1 MRS6 SEVENTASKS MPCMOEA POP2 GEN10 BIN4 LAY2 VISUALIZE'])

    # Debug settings for a neural network with multiple layers
    if(False):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['RLS MRV2 MRS6 SEVENTASKS MPCMOEA POP2 GEN10 BIN4 LAY2 NOGL STEP5 TRIAL1'])

    # Settings for the improved NSGA-II experiments
    if(False):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['RLS GEN10 POP4 MRV1 MRS2 NPAT NPCO DIVMINAVG NOMPI'])

    # Settings for loading 2016 CMOEA results
    if(False):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['DBG VISUALIZE GEN5000 POP1000 BIN100 MRV1 MRS1 NOMPI BULLET_2_81 PNGW RECORD',
                    'DBG VISUALIZE GEN5000 POP1000 BIN100 MRV1 MRS1 NOMPI BULLET_2_81 PNGW',
                    'DBG VISUALIZE GEN5000 NSGA POP6300 BIN100 MRV1 MRS2 NOMPI BULLET_2_81 PNGW RECORD',
                    'DBG VISUALIZE GEN5000 NSGA POP6300 BIN100 MRV1 MRS2 NOMPI BULLET_2_81 PNGW',
                    'DBG VISUALIZE GEN5000 LEXICASE POP6300 BIN100 MRV1 MRS2 NOMPI PNGW RECORD',
                    'DBG VISUALIZE GEN5000 LEXICASE POP6300 BIN100 MRV1 MRS2 NOMPI PNGW',
                    'DBG VISUALIZE GEN5000 NSGA POP6300 BIN100 MRV1 MRS3 NOMPI PNGW RECORD',
                    'DBG VISUALIZE GEN5000 NSGA POP6300 BIN100 MRV1 MRS3 NOMPI PNGW',
                    ])


    # Settings for the lexicase experiment
    if(True):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['RLS LEXICASE GEN10 POP4 MRV1 MRS2 NPAT DIVMINAVG NOMPI NOGL'])

    # Settings for visualizing the neural network structure
    if(False):
        sferes.create_variants(bld,
        source = 'mod_robot.cpp',
        use = 'sferes2 lzw OpenGLSupport oglft',
        uselib = uselib_,
        target = 'robot',
        cxxflags = cxxflags_kdtree,
        variants = ['DBG RENDERNETWORK GEN5000 POP1000 BIN100 MRV1 HNN SIXTASKS MRMULT SIGH MAPELITEINOVPNSGA DIVMINAVG NOMPI CIRCLE NX5 NY6'])
