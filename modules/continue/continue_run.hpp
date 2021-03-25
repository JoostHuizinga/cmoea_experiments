/*
 * continue_run.hpp
 *
 *  Created on: Aug 14, 2014
 *      Author: Joost Huizinga
 *
 * Header file that should allow for the continuation of runs.
 * Designed to require only a minimal amount of editing in other files.
 *
 * version 1.1
 *
 * QUICK GUIDE FOR NSGA2
 *
 * 1. Include continue_run.hpp and continue_ea_nsga.hpp in your cpp file:
 *  #include "modules/continue/continue_run.hpp"
 *  #include "modules/continue/continue_ea_nsga2.hpp"
 *  #include "modules/continue/stat_store_pop.hpp"
 *  #include <modules/continue/stat_merge_one_line_gen_files.hpp> //(optional)
 *
 * 2. Add the checkpoint period to the `struct pop' parameters:
 *  static const int checkpoint_period = 100;
 *
 * 3. Initialize the continue module at the start of your main function:
 *  cont::init();
 *
 * 4. Add stat::StorePop as the first element in the boost fusion vector:
 *  typedef boost::fusion::vector<stat::StorePop<phen_t, Params> >  stat_t;
 *
 * 5. Wrap your evolutionary algorithm into a variant that supports continue:
 *  typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
 *
 * 6. Replace run_ea with:
 *  options::parse_and_init(argc, argv, false);
 *  //Set your seed here
 *  cont::Continuator<ea_t, typename ea_t::params_t> continuator;
 *  continue_ea_t cont_ea;
 *  continuator.run(cont_ea);
 *
 * 7. (optional) Do some cleanup when a run finishes successfully:
 *  if(continuator.finished()){
 *      //Merge the (hypothetical) output file "bestfit.dat".
 *      //The stat::mergeOneLineGenFiles function assumes that:
 *      // - There is one line per generation
 *      // - The first number on each line indicates the generation
 *      //WARNING: DO NOT USE IT ON OTHER FILES, THIS FUNCTION WILL DELETE,
 *      //         OVERWRITE, AND MANGLE YOUR DATA!
 *      stat::mergeOneLineGenFiles(cont_ea.res_dir(), "bestfit.dat");
 *      //Deletes any empty directories
 *      io::deleteEmptyDirectories();
 *  }
 *
 * DETAILED GUIDE
 *
 * To use this header you will have to:
 * 1. Include and call functions from this header file in your cpp file
 * 2. Extend or use a stat object with a getPopulation function
 *
 *
 * 1. Include and call functions from this header in your cpp file.
 * In your .cpp file you should include, at the top of the file:
 *  #include "continue_run.hpp"
 *
 * In the parameter struct `struct pop' you will have to add:
 *  static const int checkpoint_period = 100;
 * Where the 100 should be the dump period appropriate for your experiment.
 * You can also make the variable non constant, but in that case you can not
 * initialize it here (the = 100 should be removed), and you should not forget
 * to the define it elsewhere in your .cpp file like this:
 *  size_t Params::pop::checkpoint_period = 100;
 *
 * Next, at the top of your main function, you will have to call:
 *  cont::init();
 *
 * You will also have to wrap your ea into a continue ea, which implements the
 * following functions:
 *  void set_gen(size_t gen)
 *  write_gen_file(const std::string& prefix)
 *  init_parent_pop(std::vector<boost::shared_ptr<LocalPhen> > population)
 * If you use a nsga based ea you can use the ContinueNsga2 ea, found in
 * continue_ea_nsga.hpp:
 *  typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
 *
 * Then, you will have to replace the relevant instances of run_ea with:
 *  options::parse_and_init(argc, argv, false);
 *  //Set your seed here
 *  typedef ea_t::params_t ea_params_t;
 *  cont::Continuator<continue_ea_t, ea_params_t> continuator;
 *  continue_ea_t cont_ea;
 *  continuator.run(cont_ea);
 *
 * Lastly, make sure the first element in the boost fusion vector implements
 * getPopulation().
 * For example, if your fusion vector looks like this:
 *  typedef boost::fusion::vector <stat::ModRobot<phen_t, Params> >  stat_t;
 * You have to make sure that ModRobot implements the getPopulation() function.
 *
 * If the element that implements the getPopulation() function is a different
 * one, you can supply the correct number as the third template argument to the
 * Continuator. For example, let's say you fusion vector looks like this:
 *  typedef boost::fusion::vector <
 *              stat::DoesNotImplementGetPopulation<phen_t, Params>,
 *              stat::ImplementsGetPopulation<phen_t, Params> >  stat_t;
 * You then want to define the continuator like this:
 *  cont::Continuator<ea_t, typename ea_t::params_t, 1> continuator;
 *
 *
 * 2. Extend or use a stat object with a getPopulation function.
 *
 * Lastly you have to make sure that you are using a stat object with a
 * getPopulation() function. This stat object has to store the population when
 * refresh() is called, and it should serialize the entire population of your
 * ea. All the getPopulation() function then has to do is to return this
 * population. An example implementation would be:
 *
 * 	pop_t& getPopulation(){
 *		return _all;
 *	}
 */

#ifndef CONTINUE_RUN_HPP_
#define CONTINUE_RUN_HPP_

#include <iostream>
#include <iterator>
#include <set>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/include/vector.hpp>

// Fix for a bug in boost 1.55
// See: https://codeyarns.com/2017/09/20/undefined-reference-to-boost-copy_file/
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <boost/archive/archive_exception.hpp>

#include <sferes/dbg/dbg.hpp>
#include <modules/debugext/dbgext.hpp>
#include <modules/datatools/io_convenience.hpp>

#include "global_options.hpp"

#define DBO dbg::out(dbg::info, "continue")

using namespace dbgext;
namespace bf = boost::filesystem;

namespace sferes{
namespace cont{

//Brings the << operator into scope for vectors and sets
using namespace sferes::io;
typedef std::vector<std::string> vec_str_t;


/**
 * Returns the generation number of a gen file filename.
 *
 * @param filename  The filename of a gen file.
 *                  The filename has to have the form
 *                  "{prefix}{generationNumber}"
 *                  For example: "gen_1000"
 *                  Where "gen_" is the {prefix} and "1000" is the
 *                  {generationNumber}
 * @param genPrefix The prefix of the gen file, usually "gen".
 * @return          The generation number.
 *                  Returns -1 if the filename did not match the prefix,
 *                  or if the generation number is invalid (not a number,
 *                  negative, a float, ect.).
 */
int getGenerationNumber(const std::string& filename, const std::string& prefix){
	dbg::trace trace("continue", DBG_HERE);
	int result;
	std::string genNumber;
	if(!(filename.substr(0, prefix.size()) == prefix)){
		return -1;
	}

	genNumber = filename.substr(prefix.size());
	try{
		result = boost::lexical_cast<int>(genNumber);
	} catch(boost::bad_lexical_cast&){
		result = -1;
	}

	if(result < -1){
		result = -1;
	}

	return result;
}

/**
 * Returns the generation of a gen file (if the supplied file is indeed a gen
 * file). Overloaded function.
 *
 * @param genPath   A boost system path to a gen file.
 *                  The gen path has to have the form
 *                  "{/path/to/file/}{prefix}{generationNumber}"
 *                  For example: "/project/myExperiment/run_1/gen_1000"
 *                  Where "/project/myExperiment/run_1/" is the {/path/to/file/}
 *                  "gen_" is the {prefix} and "1000" is the {generationNumber}
 * @param genPrefix The prefix of the gen file, usually "gen".
 * @return          The generation number.
 *                  Returns -1 if the filename did not match the prefix,
 *                  or if the generation number is invalid (not a number,
 *                  negative, a float, ect.).
 */
int getGenerationNumber(const bf::path& genPath, const std::string& genPrefix){
	dbg::trace trace("continue", DBG_HERE);
	std::string genFilename (genPath.filename().string());
	return getGenerationNumber(genFilename, genPrefix);
}

/**
 * Returns the generation of a gen file (if the supplied file is indeed a gen
 * file). Overloaded function.
 *
 * @param filename The filename of a gen file.
 *                 The filename has to have the form
 *                 "{prefix}{generationNumber}"
 *                 For example: "gen_1000"
 *                 Where "gen_" is the {prefix} and "1000" is the
 *                 {generationNumber}
 * @param prefixes A vector of prefixes that are possible for gen files
 *                 (usually "gen_" and "checkpoint_").
 * @return          The generation number.
 *                  Returns -1 if the filename did not match any prefixes.
 */
int getGenerationNumber(const std::string& filename, const vec_str_t& prefixes){
	dbg::trace trace("continue", DBG_HERE);
	int generation = -1;
	for(int i = 0; i < prefixes.size() && generation == -1; ++i){
		generation = getGenerationNumber(filename, prefixes[i]);
	}
	return generation;
}


/**
 * Returns the generation of a gen file (if the supplied file is indeed a gen
 * file). Overloaded function.
 *
 * @param genPath   A boost system path to a gen file.
 *                  The gen path has to have the form
 *                  "{/path/to/file/}{prefix}{generationNumber}"
 *                  For example: "/project/myExperiment/run_1/gen_1000"
 *                  Where "/project/myExperiment/run_1/" is the {/path/to/file/}
 *                  "gen_" is the {prefix} and "1000" is the {generationNumber}
 * @param prefixes  A vector of prefixes that are possible for gen files
 *                  (usually "gen_" and "checkpoint_").
 * @return          The generation number.
 *                  Returns -1 if the filename did not match any prefixes.
 */
int getGenerationNumber(const bf::path& genPath, const vec_str_t& prefixes){
	dbg::trace trace("continue", DBG_HERE);
	std::string genFilename (genPath.filename().string());
	return getGenerationNumber(genFilename, prefixes);
}


template<typename Params>
int getNumberOfGenerations(){
	int gen = 0;
	if(options::map.count("generation")){
		gen = options::map["generation"].as<int>();
	} else {
		gen = Params::pop::nb_gen;
	}
	return gen;
}

/**
 * Functor that tests whether the supplied file is a gen file.
 */
class IsGenFileFunctor{
public:
	/**
	 * Constructs a functor that takes a string and returns true if the string
	 * matches any of the prefixes followed by a non-negative integer.
	 *
	 * @param prefixes A vector of prefixes that will be tested against.
	 */
	IsGenFileFunctor(std::vector<std::string> prefixes): _prefixes(prefixes)
	{
		//nix
	}

	/**
	 * Tests whether the filename matches any of the prefixes supplied at
	 * construction, followed by a non-negative integer.
	 *
	 * @param filename The filename to test.
	 */
	bool operator()(const std::string& filename) const{
		int gen = -1;
		for(int i = 0; i < _prefixes.size() && gen == -1; ++i){
			gen = getGenerationNumber(filename, _prefixes[i]);
		}
		return gen >= 0;
	}

private:
	std::vector<std::string> _prefixes;
};


/**
 * Functor for sorting gen files.
 */
class GenPathLessThanFunctor{
public:
	/**
	 * Constructs a functor that takes two paths and returns true if the first
	 * path is smaller than the second path. When both files have the same
	 * generation number the order of the prefixes determines which generation
	 * is considered the smaller one, where the first prefix is considered
	 * smaller than the second prefix.
	 *
	 * @param prefixes A vector of prefixes that will be tested against.
	 */
	GenPathLessThanFunctor(vec_str_t prefixes): _prefixes(prefixes)
	{
		//nix
	}

	/**
	 * Compares two paths to gen files and returns true if path1 is smaller than
	 * path2. A path is considered smaller than another path if its generation
	 * number is smaller. If the path could not be parsed its generation number
	 * is considered -1, and thus an invalid path will always be smaller than
	 * any valid path. When two paths have the same generation number ties are
	 * broken by position of their prefixes, where the prefix added first is
	 * considered the smaller one.
	 *
	 * @param path1 A boost path to the first gen file.
	 * @param path2 A boost path to the second gen file.
	 * @return      True if path1 is smaller than path2, false otherwise.
	 */
	bool operator()(const bf::path& path1, const bf::path& path2) const{
		int gen_1 = -1;
		int gen_2 = -1;
		int prf_1;
		int prf_2;

		for(prf_1 = 0; prf_1 < _prefixes.size() && gen_1 == -1; ++prf_1){
			gen_1 = getGenerationNumber(path1, _prefixes[prf_1]);
		}
		prf_1--;

		for(prf_2 = 0; prf_2 < _prefixes.size() && gen_2 == -1; ++prf_2){
			gen_2 = getGenerationNumber(path2, _prefixes[prf_2]);
		}
		prf_2--;

		if(gen_1 < gen_2){
			return true;
		} else if(gen_1 > gen_2){
			return false;
		}
		return prf_1 < prf_2;
	}


private:
	std::vector<std::string> _prefixes;
};


/**
 * The Continuator class, an object that handles continuing a run.
 */
template<typename EAType, typename Params, int getPopIndex = 0>
class Continuator{

public:
	typedef std::vector<boost::shared_ptr<typename EAType::phen_t> > pop_t;

	/**
	 * Constructs a continuator class for continuing runs.
	 *
	 * @param genPrefix        The prefix expected and used for gen files.
	 * @param checkpointPrefix The prefix expected and used for checkpoints.
	 */
	Continuator(const std::string& genPrefix = "gen_",
			const std::string& checkpointPrefix = "checkpoint_"):
		_files_to_try(2),
		_deleteCheckpoints(true),
		_dumpLastGeneration(true),
		_dumpFirstGeneration(false),
		_runContinued(false),
		_runStarted(false),
		_runEnded(false),
		_transferCommand(""),
		_searchDir("."),
		_searchDepth(1)
	{
		_allFiles.push_back(checkpointPrefix);
		_checkpointsOnly.push_back(checkpointPrefix);
		_genFilesOnly.push_back(genPrefix);
		_allFiles.push_back(genPrefix);
		_finalGeneration = -1;
	}

	void setTransferCommand(std::string command){
	    _transferCommand = command;
	}

	void setSearchDir(std::string dir){
		_searchDir = dir;
	}

	void setSearchDepth(int depth){
		_searchDepth = depth;
	}

	std::string getSearchDir(){
		return _searchDir;
	}

	/**
	 * If set, all other checkpoints will be deleted upon writing a checkpoint,
	 * and all remaining checkpoints will be deleted at the end of a run.
	 *
	 * @param deleteCheckpoints Boolean indicating whether this function should
	 *                          be enabled or not. Default is true.
	 */
	void setDeleteCheckpoints(bool deleteCheckpoints = true){
		dbg::trace trace("continue", DBG_HERE);
		_deleteCheckpoints = deleteCheckpoints;
	}

	/**
	 * If set we will dump the first generation as a gen file, otherwise it will
	 * be dumped as a checkpoint (and thus deleted). The main reason for this
	 * function is that, by using dump periods, the first generation is always
	 * dumped. If your code is deterministic this is often a huge waste, since
	 * all you need is the seed to run from the start, so this setting allows
	 * you to not dump this generation.
	 *
	 * @param dump_last_generation Boolean indicating whether to dump the last
	 *                             generation. Default is true.
	 */
	void setDumpFirstGeneration(bool dumpFirstGeneration = true){
		dbg::trace trace("continue", DBG_HERE);
		_dumpFirstGeneration = dumpFirstGeneration;
	}

	/**
	 * If set we will always dump the last generation.
	 *
	 * @param dump_last_generation Boolean indicating whether to dump the last
	 *                             generation. Default is true.
	 */
	void setDumpLastGeneration(bool dump_last_generation = true){
		dbg::trace trace("continue", DBG_HERE);
		_dumpLastGeneration = dump_last_generation;
	}

	/**
	 * Sets the number of files to try for when some files fail to load.
	 * Note, if this value is set to 0 there will be no attempt to load any
	 * files. The default value is 2.
	 *
	 * @param filesToTry The number of files to try before giving up on trying
	 *                   to continue a run.
	 */
	void setFilesToTry(size_t filesToTry){
		_files_to_try = filesToTry;
	}

	/**
	 * Sets the _genFilesOnly vector. The _genFilesOnly prefixes are used to
	 * determine which files are gen files used to check whether a run finished
	 * successfully. The last prefix in the vector will be used for writing gen
	 * files. You can do a lot funky stuff here, but it should not be necessary
	 * to use under normal circumstances.
	 *
	 * @param genPrefixes The gen prefixes.
	 */
	void setGenPrefixes(const vec_str_t& genPrefixes){
		_genFilesOnly = genPrefixes;
	}

	/**
	 * Sets the _checkpointsOnly vector. The _checkpointsOnly prefixes are used
	 * to determine which files are checkpoint files; these are the files that
	 * will get deleted at the end of the run. The last prefix in the vector
	 * will be used for writing checkpoint files. You can do a lot funky stuff
	 * here, but it should not be necessary to use under normal circumstances.
	 *
	 * @param checkpointPrefixes The checkpoint prefixes.
	 */
	void setCheckpointPrefixes(const vec_str_t& checkpointPrefixes){
		_checkpointsOnly = checkpointPrefixes;
	}

	/**
	 * Sets the _allFiles vector. The _allFiles prefixes are used to determine
	 * which files can be used to continue. The order of these prefixes will
	 * determine which files are checked first when generation number are equal,
	 * (the last prefix will be used first). You can do a lot funky stuff here,
	 * but it should not be necessary to use under normal circumstances.
	 *
	 * @param allPrefixes The all prefixes.
	 */
	void setAllPrefixes(const vec_str_t& allPrefixes){
		_allFiles = allPrefixes;
	}


	/**
	 * Starts a run with the supplied ea.
	 *
	 * @param ea The ea that will be used for the run.
	 */
	void run(EAType& ea){
	    if (options::map.count("dir")) {
	    	std::string sub_dir_name = misc::date() + "_" + misc::getpid();
	    	std::string res_dir = options::map["dir"].as<std::string>() + "/" + sub_dir_name;
	        boost::filesystem::path res_path(res_dir);
	        boost::filesystem::create_directories(res_path);
	    	ea.set_res_dir(res_dir);
	    	setSearchDir(options::map["dir"].as<std::string>());
	    }

		if(options::map.count("load")){
			DBO << "Option 'load'; don't continue." << dbe;
			loadRun(ea);

		} else if(options::map.count("no-continue")){
			DBO << "Option 'no-continue'; don't continue." << dbe;
			normalRun(ea);

		} else if(options::map.count("continue")){
			bf::path fileToLoad = options::map["continue"].as<std::string>();
			DBO << "Option 'continue'; continue: " << fileToLoad << dbe;
			continueRun(ea, fileToLoad);

		} else if(options::map.count("clean")){
			DBO << "Option 'clean'; do not run." << dbe;
            return;

		} else{
			//Find all candidate gen files and their generation
			std::vector<bf::path> sorted_paths = _getSortedPaths(_allFiles);
			DBO << "Gen files found:\n" << sorted_paths << dbe;

			//Check if gen files have been found, if not assume fresh run.
			if(sorted_paths.empty()){
				DBO << "No gen files found, assuming fresh run."<< dbe;
				normalRun(ea);
			} else {
				DBO << "Gen files found, trying to load one."<< dbe;
				bf::path fileToLoad = _getFileToLoad(sorted_paths);
				continueRun(ea, fileToLoad);
			}
		}
	}

	/**
	 * Continue the run from the supplied filename.
	 *
	 * @param ea       The ea used to continue the run.
	 * @param filename The file used to continue the run.
	 */
	void continueRun(EAType& ea, const bf::path& filename){
	        dbg::trace trace("continue", DBG_HERE);
		if(filename.empty()){
			DBO << "File to continue not provided (maybe " <<
					"_getFileToLoad() did not find a suitable file), " <<
					"aborting."<< dbe;
			return;
		}
		// The old method for continuing
		//ea.init_parent_pop(_getPopulationFromFile(ea, filename));
		ea.set_pop(_getPopulationFromFile(ea, filename));
		int start = getGenerationNumber(filename, _allFiles) + 1;
		_runContinued = true;
		startRun(ea, start);
	}

	/**
	 * Start a fresh run with a random population.
	 *
	 * @param ea The ea used to start the run.
	 */
	void normalRun(EAType& ea){
		dbg::trace trace("continue", DBG_HERE);
		ea.random_pop();
		startRun(ea);
	}

	/**
	 * Load a gen file and perform analysis.
	 *
	 * @param ea The ea used to load the run.
	 */
	void loadRun(EAType& ea){
		dbg::trace trace("continue", DBG_HERE);

		std::string output_file =  options::map["out"].as<std::string>();
		int stat = options::map["stat"].as<int>();
		int n = options::map["number"].as<int>();

		std::cout << "Loading: " << options::map["load"].as<std::string>() << std::endl;
		std::cout << "stat: " << stat << " number: " << n << " output " << output_file << std::endl;

		ea.load(options::map["load"].as<std::string>());

		std::ofstream ofs(output_file.c_str());
		ea.show_stat(stat, ofs, n);

	}


    /**
     * Starts a run from the indicated generation.
     *
     * @param ea    The ea used to start the run.
     * @param start The generation at which to start.
     *              Default is 0.
     */
    void startRun(EAType& ea, int start = 0){
        dbg::trace trace("continue", DBG_HERE);

        int finalGeneration = _getFinalGeneration();
        DBO << "Running from generation " << start <<
        		" to generation " << finalGeneration << dbe;
        if(ea.res_dir().empty()) ea.set_res_dir("");
	    _runStarted = start < finalGeneration;
        for (int _gen = start; _gen < finalGeneration; ++_gen)
        {
            dbg::out(dbg::info, "gen") << "Generation: " << _gen << std::endl;
            ea.set_gen(_gen);

            dbg::out(dbg::info, "gen") << "Epoch..." << std::endl;
            ea.epoch();

            dbg::out(dbg::info, "gen") << "Updating stats..." << std::endl;
            ea.update_stats();

            dbg::out(dbg::info, "gen") << "Writing checkpoint..." << std::endl;
            if (_timeToDump(_gen)){
                if(_gen > 0 || _dumpFirstGeneration){
                	// Write a gen file
                    _writeCheckpoint(ea, _genFilesOnly.back());
                } else {
                    // Write a checkpoint
                    _writeCheckpoint(ea, _checkpointsOnly.back());
                }
            } else if(_timeToCheckpoint(_gen)){
                // Write a checkpoint
                _writeCheckpoint(ea, _checkpointsOnly.back());
#if defined(BARRIER_BEFORE_DELETE)
                ea.barrier();
#endif
                deleteCheckpoints(true);
            }
        }

        //Dump last generation
        if(_dumpLastGeneration){
        	dbo("gen") << "Dumping last generation" << dbe;
            std::vector<bf::path> paths = _getSortedPaths(_allFiles);
            if(paths.empty()){
            	dbo("gen") << "No checkpoints found" << dbe;
            	dbo("gen") << "Writing final gen file." << dbe;
                _writeCheckpoint(ea, _genFilesOnly.back());
            } else if(!_isFinalGeneration(paths.back())){
            	dbo("gen") << "Most recent checkpoint is not final." << dbe;
            	dbo("gen") << "Writing final gen file." << dbe;
                _writeCheckpoint(ea, _genFilesOnly.back());
            } else if(getGenerationNumber(paths.back(), _genFilesOnly) == -1){
            	dbo("gen") << "Most recent checkpoint is final" << dbe;
                std::string finalGenStr = str(finalGeneration - 1);
                std::string filename = _genFilesOnly.back() + finalGenStr;
                bf::path newFileName(filename);
                bf::path target = paths.back().parent_path()/newFileName;
                dbo("gen") << "Renaming file from " << paths.back() <<
                		" to: " << target << dbe;
                boost::filesystem::rename(paths.back(), target);
            }
        }

        _runEnded = true;

        //Delete all checkpoints
        deleteCheckpoints(false);
    }

	/**
	 * Checks if it is appropriate to perform cleanup (if any).
	 *
	 * Cleanup is appropriate if:
	 * - We finished a continued (as opposed finishing a run from scratch).
	 * - The latest gen file is final and loadable (otherwise we may need some
	 *   of the files we might cleanup to recover).
	 * - The --no-clean option is not provided.
	 *
	 * performCleanup will also return true of the --clean option is provided,
	 * even if we did not finish a continued run. However, all other conditions
	 * (e.g. the latest gen files is final and loadable) still need to hold.
	 *
	 * @return Returns true if cleanup is appropriate.
	 */
	bool performCleanup(){
		dbg::trace trace("continue", DBG_HERE);
        if(options::map.count("no-clean")){
        	DBO << "Option 'no-clean'; don't clean-up." << dbe;
            return false;
        }

		if(!_runContinued && !options::map.count("clean")){
			DBO << "We did not continue; don't clean-up." << dbe;
			return false;
		}

		std::vector<bf::path> paths = _getSortedPaths(_genFilesOnly);
		if(paths.empty()){
			DBO << "No gen files found; don't clean-up." << dbe;
			return false;
		}

		if(!_isFinalGeneration(paths.back())){
			DBO << "Last gen file is not final; don't clean-up." << dbe;
			return false;
		}

		if(!_isComplete(paths.back())){
			DBO << "Last gen file incomplete; don't clean-up." << dbe;
			return false;
		}

		DBO << "Last gen file is final and complete; clean-up." << dbe;
		return true;
	}

	/**
	 * Checks that we properly started and finished a run.
	 *
     * @return Returns true if we started and finished a run. False otherwise.
	 */
	bool endOfRun(){
	    return _runStarted && _runEnded;
	}

	/**
	 * Deletes all checkpoints, possibly keeping the most recent checkpoint.
	 * The function will abort if:
	 * - _deleteCheckpoints is set to false.
	 * - no checkpoint files are found.
	 * - the most recent checkpoint is not complete (meaning something went
	 * wrong, we might need those checkpoints).
	 *
	 * @param keepMostRecentChecpoint Boolean indicating whether to keep the
	 *                                most recent checkpoint. Default is false.
	 */
	void deleteCheckpoints(bool keepMostRecentChecpoint = false){
		dbg::trace trace("continue", DBG_HERE);
		DBO  << "Delete checkpoints:" << dbe;
		if(!_deleteCheckpoints){
			DBO  << "  deleting checkpoints disabled, aborting" << dbe;
			return;
		}

		std::vector<bf::path> paths = _getSortedPaths(_checkpointsOnly);
		DBO  << "  paths found:\n" << paths << dbe;

		if(paths.empty()){
			DBO  << "  no paths found, aborting." << dbe;
			return;
		}

		if(!_isComplete(paths.back())){
			DBO  << "  last checkpoint not complete, aborting." << dbe;
			return;
		}

		if(keepMostRecentChecpoint){
			for(size_t i=0; i<paths.size()-1; ++i){
				DBO  << "  deleting all but last:\n" << paths[i] << dbe;
				boost::filesystem::remove(paths[i]);
			}
		}else{
			for(size_t i=0; i<paths.size(); ++i){
				DBO  << "  deleting all:\n" << paths[i] << dbe;
				boost::filesystem::remove(paths[i]);
			}
		}
	}

	void moveGenFiles(const std::string& targetDir){
        dbg::trace trace("continue", DBG_HERE);

        std::vector<bf::path> paths = _getSortedPaths(_genFilesOnly);
        DBO << "Move gen files: paths found:\n" << paths << dbe;

        if(paths.size() < 1){
        	DBO << "Move gen files: " << paths.size() <<
        			" paths found, at least 1 paths " <<
					"required for moving. Aborting." << dbe;
            return;
        }

        bf::path targetDirectory = bf::path(targetDir);

        for(size_t i=0; i<paths.size(); ++i){
            bf::path origin = paths[i];
            bf::path target = targetDirectory/origin.filename();
            if(bf::equivalent(origin, target)){
            	DBO << "Origin and target are " <<
                		"the same: don't copy and don't delete." << dbe;
            	DBO << " - origin: " << origin << dbe;
            	DBO << " - target: " << target << dbe;
            } else if(boost::filesystem::exists(target)){
                //If the file already exists on the target location, just remove
            	//the origin
            	DBO << "Target already exists. " <<
                		"Doing a quick and dirty check to see if the origin " <<
						"a target may be the same." << std::endl;
                uintmax_t size_origin = bf::file_size(origin);
                uintmax_t size_target = bf::file_size(target);
                DBO << " - origin: " << origin <<
                		" size: " << size_origin << std::endl;
                DBO << " - target: " << target <<
                		" size: " << size_target << std::endl;
                if(size_origin == size_target || size_origin == 0){
                	DBO << "Origin size and target size are the " <<
                			"same or origin size is 0. Removing origin." << dbe;
                try{
                    bf::remove(origin);
                } catch (const bf::filesystem_error& e){
                    std::cerr << "Error: " << e.what() << std::endl;
                }
                }
            } else{
            	DBO << "Origin exists and target does not, copy the " <<
            			"origin to the target." << dbe;
            	DBO << " - origin: " << origin << dbe;
            	DBO << " - target: " << target << dbe;
                //Otherwise, try to copy the file to the target location, then
                //remove the origin
                try{
                    bf::copy_file(origin, target);
                    bf::remove(origin);
                } catch (const bf::filesystem_error& e){
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
        }
	}

private:
	size_t _files_to_try;
	std::vector<std::string> _allFiles;
	std::vector<std::string> _checkpointsOnly;
	std::vector<std::string> _genFilesOnly;
	bool _deleteCheckpoints;
	bool _dumpLastGeneration;
	bool _dumpFirstGeneration;
	bool _runContinued;
	bool _runStarted;
	bool _runEnded;
	int _finalGeneration;
	std::string _transferCommand;
	std::string _searchDir;
	int _searchDepth;

	bool _timeToDump(int gen){
		if (Params::pop::dump_period == 0) return true;
		if (gen % Params::pop::dump_period == 0) return true;
		return false;
	}

	bool _timeToCheckpoint(int gen){
		if (Params::pop::checkpoint_period == 0) return true;
		if (gen % Params::pop::checkpoint_period == 0) return true;
		return false;
	}
	/**
	 * Gets the population from the _fileToLoad.
	 *
	 * @param ea   The ea that will be used to load the population.
	 * @param file The file to get the population from.
	 * @return     The population.
	 */
	pop_t _getPopulationFromFile(EAType& ea, const bf::path& file){
		dbg::trace trace("continue", DBG_HERE);
		ea.load(file.string());
		return boost::fusion::at_c<getPopIndex>(ea.stat()).getPopulation();
	}

	/**
	 * Checks if the file is a completed archive file by checking the last line.
	 *
	 * @param path The path to the file to check.
	 * @return     Returns true if the file is complete, and false otherwise.
	 */
	bool _isComplete(const boost::filesystem::path& path){
		dbg::trace trace("continue", DBG_HERE);
		if(options::map.count("no-check-complete")){
			DBO << "Argument 'no-check-complete' provided, do not " <<
					"check whether file is complete." << dbe;
		    return true;
		}

		std::string stl_line = io::getLineFromBack(path, 2);
		bool result = stl_line == "</boost_serialization>";
		DBO << "Is complete: second to last line: " << stl_line
		            << " compared to </boost_serialization> gives the value: "
					<< result << dbe;
		return result;
	}


	/**
	 * Returns a vector of paths sorted from the oldest checkpoint to the most
	 * recent checkpoint.
	 *
	 * @param prefixes Possible prefixes for the files.
	 * @return         A vector of matching gen and checkpoint files, sorted in
	 *                 ascending order.
	 */
	std::vector<bf::path> _getSortedPaths(const vec_str_t& prefixes){
		dbg::trace trace("continue", DBG_HERE);
		IsGenFileFunctor is_gen_file(prefixes);
		std::set<bf::path> paths = io::find(_searchDir, is_gen_file,
				_searchDepth, _searchDepth);
		std::vector<bf::path> result(paths.begin(), paths.end());
		GenPathLessThanFunctor compare(prefixes);
		std::sort(result.begin(), result.end(), compare);
		return result;
	}

	/**
	 * Checks if the checkpoint in the supplied path is the final checkpoint.
	 *
	 * @param path The supplied path.
	 */
	bool _isFinalGeneration(boost::filesystem::path path){
		dbg::trace trace("continue", DBG_HERE);
		int generation = getGenerationNumber(path, _allFiles);
		return generation == _getFinalGeneration()-1;
	}

	/**
	 * Search the sorted paths for a suitable file to load.
	 *
	 * @params sorted_paths Paths to possible files to load.
	 *                      These paths are assumed to be sorted ascending,
	 *                      and the last path will be tried first.
	 * @return              Returns a path to the file to load, or an empty path
	 *                      if no such file was found.
	 */
	bf::path _getFileToLoad(std::vector<bf::path>& sorted_paths){
		dbg::trace trace("continue", DBG_HERE);
		bf::path result;

		//Check if the final generation file exists and that it is loadable. If
		//so, assume run finished successfully.
		DBO << "Max generation file: " << sorted_paths.back() << dbe;
		bool final = _isFinalGeneration(sorted_paths.back());
		bool complete = _isComplete(sorted_paths.back());
		if(final && complete){
			DBO << "Max generation file is loadable, "
						<< "and it is the last generation requested. "
						<< "Assuming run finished successfully."<< dbe;
			return result;
		}

		//Try loading files, most recent ones first
		for(int i=0; i<_files_to_try && !sorted_paths.empty(); ++i){
			DBO << "Attempt " << i << " "
					    << "file to try: " << sorted_paths.back() << dbe;
			if(_isComplete(sorted_paths.back())){
				result = sorted_paths.back();
				DBO << "File is loadable, "
						    << "and not the final generation. "
							<< "Continuing from file:" << result << dbe;
				return result;
			}
			sorted_paths.pop_back();
		}

		//No complete gen files were found, aborting.
		DBO << "No complete gen files found, aborting." <<dbe;
		return result;
	}

	/**
	 * Returns the final generation, either from params, or from the arguments.
	 *
	 * @return The generation provided by the --generation option, or
	 *     Params::pop::nb_gen if no generation was provided.
	 */
	int _getFinalGeneration(){
	    if(_finalGeneration == -1){
	        if(options::map.count("generation")){
	            _finalGeneration = options::map["generation"].as<int>();
	        } else {
	            _finalGeneration = Params::pop::nb_gen;
	        }
	    }
	    return _finalGeneration;
	}

	/**
	 * Writes a checkpoint (or gen file) to the disk, and potentially calls an
	 * external transfer command.
	 *
	 * @param ea   The evolutionary algorithm
	 * @param prfx The prefix of the checkpoint file that needs to be written
	 */
	void _writeCheckpoint(EAType& ea, std::string& prfx){
	  //boost::fusion::at_c<getPopIndex>(ea.stat()).setPopulation(ea);
	    std::string file_name = ea.res_dir() + str("/") + prfx + str(ea.gen());
	    ea.write_gen_file(prfx);
#ifdef OSG
	    if(_transferCommand != "") {
	        bf::path last_chkpt(file_name);
	        std::set<bf::path> dir_contents = getDirContents(ea.res_dir());
	        BOOST_FOREACH(bf::path path, dir_contents){
	            // First copy everything else, because a successfully copied
	        	// checkpoint signals that all files have transferred correctly
	        	std::string chkpt = _checkpointsOnly.back();
	        	std::string genfl = _genFilesOnly.back();
	        	std::string filename = path.filename().string();
	            bool is_chkpt = filename.compare(0, chkpt.size(), chkpt) == 0;
	            bool is_genfl = filename.compare(0, genfl.size(), genfl) == 0;
	            DBO << "Considering for transfer: " << path << dbe;
	            DBO << "Is checkpoint: " << is_chkpt << dbe;
	            DBO << " is gen file: " << is_genfl << dbe;
	            if(!is_chkpt && !is_genfl){
	            	DBO << "Transferring file in output directory "
	            			<< path.string() << dbe;
	                std::string cmd = _transferCommand  + " " + path.string();
                    std::cout << "Executing command: " << cmd.c_str() << dbe;
	                system(cmd.c_str());
	            }
	        }
	        //Lastly, transfer the most recent checkpoint
	        DBO << "Transferring last checkpoint "
	        		<< last_chkpt.string() << dbe;
            std::string cmd = _transferCommand  + " " + last_chkpt.string();
            std::cout << "Executing command: " << cmd.c_str() << std::endl;
            system(cmd.c_str());
	    }
#endif
	}
};

/**
 * Adds options specific to the continue module.
 */
void init(){
	//Tracing init is useless, as it is called before the debugger is enabled.
	options::add()("continue,t",
			boost::program_options::value<std::string>(),
			"continue the loaded file starting from the generation provided");
	options::add()("no-continue",
			"do not try to continue this run, no matter what files we find");
    options::add()("no-check-complete",
            "do not check whether gen files are complete");
    options::add()("clean",
            "do not run, but do attempt to perform clean-up");
    options::add()("no-clean",
            "run, but do not attempt to perform clean-up");
    options::add()("generation,g",
            boost::program_options::value<int>(),
            "run until the indicated generation");
    options::add()("dir,d",
    		boost::program_options::value<std::string>(),
			"custom directory for gen files (when evolving)");

}

}
}

#undef DBO

#endif /* CONTINUE_RUN_HPP_ */
