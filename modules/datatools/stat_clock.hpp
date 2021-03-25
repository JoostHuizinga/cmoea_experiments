// stat_clock.hpp
#ifndef MODULES_MISC_STAT_CLOCK_HPP_
#define MODULES_MISC_STAT_CLOCK_HPP_

// Sferes includes
#include <sferes/stat/stat.hpp>
#include <sferes/dbg/dbg.hpp>

// Module includes
#include <modules/datatools/clock.hpp>
#include <modules/datatools/stat_printer.hpp>

namespace sferes
{
namespace stat
{


SFERES_STAT(StatClock, Stat)
{
	typedef std::map<std::string, Clock> clocks_t;
	typedef typename std::map<std::string, Clock>::iterator clocks_it_t;
public:
    template<typename E>
    void refresh(const E& ea){
        dbg::trace trace("stat", DBG_HERE);
        if (ea.gen() % Params::stats::period != 0) return;
        dbg::out(dbg::info, "stat") << "Creating log file..." << std::endl;
        this->_create_log_file(ea, "clock.dat");
        clocks_t clocks = ea.getClocks();
        dbg::out(dbg::info, "stat") << "Writing..." << std::endl;
        _printer.dataStart();
        _printer.add(true, "#gen", ea.gen());
        for(clocks_it_t it = clocks.begin(); it != clocks.end(); it++) {
        	_printer.add(true, it->first, it->second.time());
        }
        _printer.dataPrint(this->_log_file);
        dbg::out(dbg::info, "stat") << "Refresh finished" << std::endl;
    }


    std::string name(){
        return "StatClock";
    }

protected:
    StatPrinter _printer;
};
}
}


#endif //MODULES_MISC_STAT_CLOCK_HPP_
