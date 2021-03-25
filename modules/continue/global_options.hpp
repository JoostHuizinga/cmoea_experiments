/*
 * global_options.hpp
 *
 *  Created on: Aug 14, 2014
 *      Author: Joost Huizinga
 *
 *  Header file created to allow for custom command-line options to be added to an experiment.
 *  Requires you to replace run_ea with options::run_ea to works.
 *
 *  Options can be added with:
 *  options::add()("option_name","description");
 * 
 *  Options can be queried with:
 *  options::map["option-name"].as<type>()
 */

#ifndef GLOBAL_OPTIONS_HPP_
#define GLOBAL_OPTIONS_HPP_

// Standard includes
#include <fstream>
#include <iostream>

// Boost includes
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/foreach.hpp>

// Sferes includes
#include <sferes/eval/parallel.hpp>
#include <sferes/dbg/dbg.hpp>

namespace sferes{
namespace options{

boost::program_options::options_description options_description;
boost::program_options::positional_options_description positional_options_description;
boost::program_options::variables_map map;

/**
 * Allows you to add an option according to the boost add option functionality.
 */
boost::program_options::options_description_easy_init add(){
    return options_description.add_options();
}

/**
 * Allows you to add a positional option.
 */
void positionalAdd(std::string name, int position){
    positional_options_description.add(name.c_str(), position);
}

/**
 * Parses the command line arguments and sets the global options.
 *
 * Should be called before options::run_ea(ea).
 *
 * @param argc      The number of command line arguments.
 * @param argv      The array of character arrays that hold the command line options.
 * @param init_rand Initializes the random seed. Added for backwards compatibility with ea_run,
 *                  but this functionality should probably be removed.
 */
static void parse_and_init(int argc, char **argv, bool init_rand)
{
    namespace po = boost::program_options;

    // Checking for both SVN_VERSION and VERSION for backwards compatibility
#if defined(SVN_VERSION)
    std::cout<<"sferes2 svn version: "<< SVN_VERSION << std::endl;
#elif defined(VERSION)
    std::cout<<"sferes2 version: "<< VERSION << std::endl;
#endif

    //Honestly, this method of setting the seed should be deprecated
    if (init_rand)
    {
        time_t t = time(0) + ::getpid();
        std::cout<<"seed: " << t << std::endl;
        srand(t);
    }

    //Add the default options
    options_description.add_options()
            ("help,h", "produce help message")
			("load,l", po::value<std::string>(), "load a result file")
            ("stat,s", po::value<int>()->default_value(0), "statistic number")
            ("out,o", po::value<std::string>()->default_value("indiv.log"), "output file (when loading)")
            ("number,n", po::value<int>()->default_value(-1), "number in stat (when loading)")
            ("dbg_debug", po::value<bool>()->default_value(false), "debug information about the debugger")
            ("verbose,v", po::value<std::vector<std::string> >()->multitoken(),
                    "verbose output, available default streams : all, ea, fit, phen, trace")
                    ;

    //Parse options
    boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv).options(options_description).positional(positional_options_description).run(),
            map
    );
    boost::program_options::notify(map);

    //Initialize the debugger
    dbg::init();
    attach_ostream(dbg::warning, std::cout);
    attach_ostream(dbg::error, std::cerr);
    attach_ostream(dbg::info, std::cout);

    //Process the basic options 'help' and 'verbose'
    if (map.count("help"))
    {
        std::cout << options_description << std::endl;
        exit(0);
    }
    bool dbg_debug = map["dbg_debug"].as<bool>();
#if defined(NDEBUG) or not defined(DBG_ENABLED)
    if(dbg_debug) std::cout << "Debugger is not enabled!" << std::endl;
#endif
    if (map.count("verbose"))
    {
        std::vector<std::string> streams = map["verbose"].as<std::vector<std::string> >();

        bool all = std::find(streams.begin(), streams.end(), "all") != streams.end();
        bool trace = std::find(streams.begin(), streams.end(), "trace") != streams.end();
        if (all)
        {
            streams.push_back("ea");
            streams.push_back("fit");
            streams.push_back("phen");
            streams.push_back("eval");
        }
        BOOST_FOREACH(const std::string& s, streams)
        {
            if(dbg_debug) std::cout << "Printing stream: " << s << std::endl;
            dbg::enable(dbg::all, s.c_str(), true);
            dbg::attach_ostream(dbg::info, s.c_str(), std::cout);
            if (trace)
                dbg::attach_ostream(dbg::tracing, s.c_str(), std::cout);
        }
        if (trace){
            attach_ostream(dbg::tracing, std::cout);
            std::cout << "Trace is enabled" << std::endl;
        } else if(dbg_debug){
            std::cout << "Trace is not enabled" << std::endl;
        }
        std::cout << "Debugger is now active" << std::endl;
#if defined(NDEBUG) or not defined(DBG_ENABLED)
        std::cout << "WARNING: Binary was not compiled with debug flags!" << std::endl;
        std::cout << "WARNING: Debug output will be unavailable!" << std::endl;
#endif
    } else if(dbg_debug){
        std::cout << "No --verbose options provided." << std::endl;
    }

    //Initialize parallel
    dbg::out(dbg::info, "ea") << "Initializing parallel engine..." << std::endl;
    parallel::init();
    dbg::out(dbg::info, "ea") << "Parallel engine initialized." << std::endl;
}


/**
 * A variant of run_ea that allows for global options.
 *
 * To be called after parse_and_init(int argc, char **argv, bool init_rand).
 *
 * @param ea The evolutionary algorithm to run.
 */
template<typename Ea>
static void run_ea(Ea& ea)
{
    namespace po = boost::program_options;

    if (map.count("load"))
    {
        ea.load(map["load"].as<std::string>());

        if (!map.count("out"))
        {
            std::cerr<<"You must specifiy an out file"<<std::endl;
            return;
        }
        else
        {
            int stat = 0;
            int n = 0;
            if (map.count("stat"))
                stat = map["stat"].as<int>();
            if (map.count("number"))
                n = map["number"].as<int>();
            std::ofstream ofs(map["out"].as<std::string>().c_str());
            ea.show_stat(stat, ofs, n);
        }
    }
    else
        ea.run();
}

/**
 * A combination of parse_and_init(argc, argv,  init_rand) and run_ea(ea),
 * to mimic the behavior of run_ea.
 *
 * To get more control over your options, call
 *   options::parse_and_init(argc, argv,  init_rand) and
 *   run_ea(ea)
 * separately.
 *
 * @param ea The evolutionary algorithm to run.
 * @param argc      The number of command line arguments.
 * @param argv      The array of character arrays that hold the command line options.
 * @param init_rand Initializes the random seed. Added for backwards compatibility with ea_run,
 *                  but this functionality should probably be removed.
 */
template<typename Ea>
static void run_ea(int argc, char **argv, Ea& ea, bool init_rand = true){
    parse_and_init(argc, argv,  init_rand);
    run_ea(ea);
}


}

}


#endif /* GLOBAL_OPTIONS_HPP_ */
