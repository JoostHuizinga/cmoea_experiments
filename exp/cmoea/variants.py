#! /usr/bin/env python
from waflib.Configure import conf
import sferes

def build(bld):
   uselib_ = 'TBB BOOST EIGEN PTHREAD MPI SDL OGLFT FREETYPE PNGWRITER'
   cxxflags_kdtree = bld.get_env()['CXXFLAGS', '-I.', '-DLIBSSRCKDTREE_HAVE_BOOST']
   
   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             variants = ['DBG JHDEBUG VISUALIZE NSGA GEN20 POP1000',
                                         'DBG JHDEBUG VISUALIZE NSGA GEN20 POP1000 DIV',
                                         'DBG JHDEBUG VISUALIZE CMOEA GEN20 POP1000 DIV',
                                         ])
   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             variants = ['RLS JHDEBUG NSGA GEN=4 POP=4 DIV',
                                         'RLS JHDEBUG CMOEA GEN=4 POP=4 DIV',
                                         ])
      
   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             variants = ['DBG JHDEBUG MANUAL NSGA GEN1 POP4 STPGOAL MULTIPLICATIVE_FITNESS'])

   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             variants = ['DBG JHDEBUG CMOEA GEN=1 POP=4 FSTART MDIST',
                                         'RLS JHDEBUG CMOEA GEN=1 POP=4 FSTART MDIST'])

   # CMOEA Bin sampling debug settings
   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             cxxflags = cxxflags_kdtree,
                             variants = ['DBG CMOEASAMP GEN=4 POP=4 BIN=4 BINS=8 MAZES=4 DIV STPGOAL OPQWALL MDIST NOEQPOP',
                                 'DBG CMOEASAMP GEN=4 POP=4 BIN=4 BINS=8 MAZES=4 DIV STPGOAL OPQWALL MDIST',
                                 'DBG CMOEASAMP GEN=4 POP=4 BIN=4 BINS=8 MAZES=4 DIV STPGOAL OPQWALL MDIST NOEQPOP NOMPI',
                                 'DBG CMOEASAMP GEN=4 POP=4 BIN=4 BINS=8 MAZES=4 DIV STPGOAL OPQWALL MDIST NOEQPOP NOMPISELECT',
                                 ])

   # Settings for visualizing the bin-sampling experiments.
   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             cxxflags = cxxflags_kdtree,
                             variants = [
                                 'RLS CMOEASAMP GEN=4 POP=1000 BIN=10 BINS=1000 MAZES=100 DIV STPGOAL OPQWALL MDIST NOMPI VISUALIZE',
                                 'DBG CMOEASAMP GEN=4 POP=1000 BIN=10 BINS=1000 MAZES=100 DIV STPGOAL OPQWALL MDIST NOMPI',
                                 ])

   # Lexicase experiment
   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             cxxflags = cxxflags_kdtree,
                             variants = ['DBG LEXICASE GEN=10 POP=8 BIN=10 BINS=1000 MAZES=10 STPGOAL OPQWALL MDIST NOEQPOP NPAT'])

   # Guided NSGA-II experiment
   if(True):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             cxxflags = cxxflags_kdtree,
                             variants = ['RLS GNSGA GEN=1001 POP=1000 BIN=10 BINS=1000 MAZES=100 STPGOAL OPQWALL MDIST',
                                         'RLS GNSGA GEN=1001 POP=1000 BIN=10 BINS=1000 MAZES=100 STPGOAL OPQWALL MDIST VISUALIZE NOMPI',
                                         'RLS GNSGA GEN=1001 POP=1000 BIN=10 BINS=1000 MAZES=100 STPGOAL OPQWALL MDIST VISUALIZE NOMPI PNGW',
                                         ])
      
   # Load NSGA-III and e-Epsilon
   if(False):
      sferes.create_variants(bld,
                             source = 'maze.cpp',
                             use = 'sferes2 fastsim',
                             target = '',
                             uselib = uselib_,
                             cxxflags = cxxflags_kdtree,
                             variants = ['RLS MRV2 ELEXICASE GEN=1001 POP=1000 BIN=10 BINS=1000 MAZES=100 STPGOAL OPQWALL MDIST NPAT NPCO NOMPI',
                                         ])

   if(False):
      sferes.create_variants(bld,
                             source = 'hard_easy_function.cpp',
                             use = 'sferes2',
                             target = 'hardeasy',
                             uselib = uselib_,
                             variants = [
                                         'RLS CMOEA',
                                         'RLS NSGA COO',
                                         'RLS NSGA',
                                         'RLS LEXICASE',
                                         'RLS GLEXICASE',
                                         'RLS ELEXICASE NPCO PMAX PMAD',
                                         'RLS GNSGA',
                                         'RLS NSGA3',])
      

   if(True):
      sferes.create_variants(bld,
                             source = 'hard_easy_function.cpp',
                             use = 'sferes2',
                             target = 'hardeasy',
                             uselib = uselib_,
                             variants = [
                                         'RLS MEAN CMOEA',
                                         'RLS MEAN NSGA COO',
                                         'RLS MEAN NSGA',
                                         'RLS MEAN LEXICASE',
                                         'RLS MEAN GLEXICASE',
                                         'RLS MEAN ELEXICASE NPCO PMAX PMAD',
                                         'RLS MEAN GNSGA',
                                         'RLS MEAN NSGA3',])
      
   if(True):
      sferes.create_variants(bld,
                             source = 'hard_easy_function.cpp',
                             use = 'sferes2',
                             target = 'hardeasy',
                             uselib = uselib_,
                             variants = ['DBG MEAN ELEXICASE NPCO PMAX PMAD'])

   # Settings for the e-lexicase parameter sweep
   if(False):
        sferes.create_variants(bld,
        source = 'hard_easy_function.cpp',
        use = 'sferes2',
        uselib = uselib_,
        target = 'hardeasy',
        variants = [
            # Static combined-target e-lexicase
            'RLS ELEXICASE NPCO',
            # Semi-Dynamic combined-target e-lexicase
            'RLS ELEXICASE NPCO PMAX',
            # Dynamic combined-target e-lexicase
            'RLS ELEXICASE NPCO PMAX PMAD',
            # Static e-lexicase
            'RLS ELEXICASE',
            # Semi-Dynamic e-lexicase
            'RLS ELEXICASE PMAX',
            # Static e-lexicase
            'RLS ELEXICASE PMAX PMAD',
                    ])

   # Settings for the e-lexicase parameter sweep
   if(False):
        sferes.create_variants(bld,
        source = 'hard_easy_function.cpp',
        use = 'sferes2',
        uselib = uselib_,
        target = 'hardeasy',
        variants = ['RLS ELEXICASE NPCO E20',
                    'RLS ELEXICASE NPCO E15',
                    'RLS ELEXICASE NPCO E10',
                    'RLS ELEXICASE NPCO E05',
                    'RLS ELEXICASE NPCO E01',
                    'RLS ELEXICASE NPCO E20 PMAX',
                    'RLS ELEXICASE NPCO E15 PMAX',
                    'RLS ELEXICASE NPCO E10 PMAX',
                    'RLS ELEXICASE NPCO E05 PMAX',
                    'RLS ELEXICASE NPCO E01 PMAX',                    
                    ])
