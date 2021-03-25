//| This file is a part of the sferes2 framework.
//| Copyright 2009, ISIR / Universite Pierre et Marie Curie (UPMC)
//| Main contributor(s): Jean-Baptiste Mouret, mouret@isir.fr
//|
//| This software is a computer program whose purpose is to facilitate
//| experiments in evolutionary computation and evolutionary robotics.
//| 
//| This software is governed by the CeCILL license under French law
//| and abiding by the rules of distribution of free software.  You
//| can use, modify and/ or redistribute the software under the terms
//| of the CeCILL license as circulated by CEA, CNRS and INRIA at the
//| following URL "http://www.cecill.info".
//| 
//| As a counterpart to the access to the source code and rights to
//| copy, modify and redistribute granted by the license, users are
//| provided only with a limited warranty and the software's author,
//| the holder of the economic rights, and the successive licensors
//| have only limited liability.
//|
//| In this respect, the user's attention is drawn to the risks
//| associated with loading, using, modifying and/or developing or
//| reproducing the software by the user in light of its specific
//| status of free software, that may mean that it is complicated to
//| manipulate, and that also therefore means that it is reserved for
//| developers and experienced professionals having in-depth computer
//| knowledge. Users are therefore encouraged to load and test the
//| software's suitability as regards their requirements in conditions
//| enabling the security of their systems and/or data to be ensured
//| and, more generally, to use and operate it in the same conditions
//| as regards security.
//|
//| The fact that you are presently reading this means that you have
//| had knowledge of the CeCILL license and that you accept its terms.


/**
 * More advanced version of mpi.hpp,
 * which sends a 'world seed' to each of the slaves to control their world,
 * and which has the option to supply a 'masterInit' and a 'slaveInit' function,
 * which will only be executed by the master process and slave processes
 * respectively
 *
 * This header file requires the following structure in you params:
 * 	struct mpi
 *	{
 *		static mpi_init_ptr_t masterInit;
 *		static mpi_init_ptr_t slaveInit;
 *	};
 *
 * These global variable need to be set, right before main, like this:
 *  mpi_init_ptr_t Params::mpi::masterInit = 0;
 *  mpi_init_ptr_t Params::mpi::slaveInit = 0;
 *
 * And the can be initialized in you main function like this:
 * 	Params::mpi::slaveInit = &slave_init;
 *	Params::mpi::masterInit = &master_init;
 *
 * Tip of the day:
 * To make sure only your master process creates a directory,
 * do the following.
 * - Make the dump_period non-constant:
 *    static const int dump_period; -> static int dump_period;
 * - Initialize the dump_period to -1, right above main:
 *    int Params::pop::dump_period = -1;
 * - Define a master_init function with the desired dump period:
 *    void master_init(){
 *        Params::pop::dump_period = Params::pop::nb_gen - 1;
 *    }
 * - Register the master_init function in main:
 *    Params::mpi::masterInit = &master_init;
 */

#ifndef EVAL_MPI_WORLD_TRACKING_HPP_
#define EVAL_MPI_WORLD_TRACKING_HPP_

#if !defined(NR_OF_MPI_TASKS)
#define NR_OF_MPI_TASKS 1
#endif

#include <fstream>
#include <string>

#include <sferes/misc.hpp>
#include <sferes/parallel.hpp>
#include <sferes/stc.hpp>

#include <boost/mpi.hpp>

#include <modules/datatools/clock.hpp>

//#ifndef BOOST_MPI_HAS_NOARG_INITIALIZATION
//#error MPI need arguments (we require a full MPI2 implementation)
//#endif

#define MPI_INFO dbg::out(dbg::info, "mpi")<<"["<<_world->rank()<<"] "

typedef void (*mpi_init_ptr_t)();

namespace sferes
{

namespace eval
{
typedef boost::mpi::communicator com_t;
typedef boost::mpi::environment env_t;
typedef boost::mpi::status status_t;
typedef boost::shared_ptr<com_t> com_ptr_t;
typedef boost::shared_ptr<env_t> env_ptr_t;

template<typename Phen>
class EvalTask{
public:
    void run(com_ptr_t _world, status_t s, env_ptr_t env){
        Phen p;
        MPI_INFO <<"[slave] [rcv...] [" << getpid()<< "]" << std::endl;
        _world->recv(0, s.tag(), p.gen());
        MPI_INFO <<"[slave] [rcv ok] " << " tag="<<s.tag()<<std::endl;
        //Clock develop_clock;
        //develop_clock.resetAndStart();
        p.develop();
        //develop_clock.stop();
        p.fit().eval(p);
        //p.fit().setDevelopTime(develop_clock.time());
        MPI_INFO <<"[slave] [send...]"<<" tag=" << s.tag()<<std::endl;
        _world->send(0, s.tag(), p.fit());
        MPI_INFO <<"[slave] [send ok]"<<" tag=" << s.tag()<<std::endl;
    }
};

template<typename Params = stc::_Params,
		typename Tasks = boost::fusion::vector<>,
		typename Exact = stc::Itself>
class MpiWorldTracking : public stc::Any<Exact>
{
public:
    MpiWorldTracking()
    {
		static char* argv[] = {(char*)"sferes2", 0x0};
		char** argv2 = (char**) malloc(sizeof(char*) * 2);
		int argc = 1;
		argv2[0] = argv[0];
		argv2[1] = argv[1];

		dbg::out(dbg::info, "mpi")<<"Initializing MPI..."<<std::endl;
		_env = env_ptr_t(new env_t(argc, argv2, true));
		dbg::out(dbg::info, "mpi")<<"MPI initialized"<<std::endl;
		_world = com_ptr_t(new com_t());
		MPI_INFO << "communicator initialized"<<std::endl;
//		_first_call = true;
		_world_seed = 0;

		if (_world->rank() == 0){
			if(Params::mpi::masterInit != 0){
				(*Params::mpi::masterInit)();
			}
		}else{
			if(Params::mpi::slaveInit != 0){
				(*Params::mpi::slaveInit)();
			}
		}
    }

	template<typename Phen>
	void eval(std::vector<boost::shared_ptr<Phen> >& pop,
			size_t begin, size_t end,
                const typename Phen::fit_t& fit_proto)
	{
		dbg::trace trace("mpi", DBG_HERE);
		if (_world->rank() == 0){
			_master_loop(pop, begin, end);
		}else{
			_slave_loop<Phen>(fit_proto);
		}
	}
	~MpiWorldTracking()
	{
		MPI_INFO << "Finalizing MPI..."<<std::endl;
		size_t exit_signal = NR_OF_MPI_TASKS;
		if (_world->rank() == 0){
		    for (size_t i = 1; i < _world->size(); ++i){
		        _world->send(i, _env->max_tag(), exit_signal);
                    }
                }
		_finalize();
	}

	boost::shared_ptr<boost::mpi::communicator> world(){
	    return _world;
	}

	boost::shared_ptr<boost::mpi::environment> env(){
        return _env;
    }

//	bool mpi_slave() const{
//		return _world->rank() != 0;
//	}

protected:
	void _finalize()
	{
		_world = com_ptr_t();
		dbg::out(dbg::info, "mpi")<<"MPI world destroyed"<<std::endl;
		_env = env_ptr_t();
		dbg::out(dbg::info, "mpi")<<"environment destroyed"<<std::endl;
	}


	template<typename Phen>
	void _master_loop(std::vector<boost::shared_ptr<Phen> >& pop,
			size_t begin, size_t end)
	{
		dbg::trace t1("mpi", DBG_HERE);
		if(begin == end) return;

		bool seed_required = false;
		for (size_t i = begin; i < end; ++i)
		{
			if(!pop[i]->gen().world_seed_set()){
				seed_required = true;
				break;
			}
		}
		if(seed_required){
			static const long max = std::numeric_limits<long>::max();
			_world_seed = sferes::misc::rand<long>(max);
			dbg::out(dbg::info, "seed") << "World seed send: " <<
					_world_seed << std::endl;
			for (size_t i = begin; i < end; ++i)
			{
				pop[i]->gen().add_world_seed(_world_seed);
			}
		}

		//Set new task
		for (size_t i = 1; i < _world->size(); ++i){
		    _world->send(i, _env->max_tag(), 0);
		}

		size_t current = begin;
		std::vector<bool> evaluated(pop.size());
		std::fill(evaluated.begin(), evaluated.end(), false); // @suppress("Ambiguous problem")
//		std::fill<std::vector<bool>::iterator>(evaluated.begin(), evaluated.end(), false);

		// first round
		for (size_t i = 1; i < _world->size() && current < end; ++i){
			MPI_INFO << "[master] [send-init...] ->" << i <<
					" [indiv="<<current<<"]"<<std::endl;
			_send(i, current, pop);
			MPI_INFO << "[master] [send-init ok] ->" << i <<
					" [indiv="<<current<<"]"<<std::endl;
			++current;
		}

		// send a new indiv each time we received a fitness
		while (current < end){
			boost::mpi::status s = _recv(evaluated, pop);
			MPI_INFO << "[master] [send...] ->" <<s.source()<<
					" [indiv="<<current<<"]"<<std::endl;
			_send(s.source(), current, pop);
			MPI_INFO << "[master] [send ok] ->" <<s.source()<<
					" [indiv="<<current<<"]"<<std::endl;
			++current;
		}

		//join
		bool done = true;
		do
		{
			dbg::out(dbg::info, "mpi")<<"joining..."<<std::endl;
			done = true;
			for (size_t i = begin; i < end; ++i)
				if (!evaluated[i])
				{
					_recv(evaluated, pop);
					done = false;
				}
		}
		while (!done);
		dbg::out(dbg::info, "mpi")<<"join complete..."<<std::endl;
		//		std::cout << "Master: " << rand() <<std::endl;
	}

	template<typename Phen>
	boost::mpi::status _recv(std::vector<bool>& evaluated,
			std::vector<boost::shared_ptr<Phen> >& pop)
	{
		dbg::trace t1("mpi", DBG_HERE);
		using namespace boost::mpi;
		status s = _world->probe();
		MPI_INFO << "[rcv...]" << getpid() << " tag=" << s.tag() << std::endl;
		_world->recv(s.source(), s.tag(), pop[s.tag()]->fit());
		MPI_INFO << "[rcv ok]" << " tag=" << s.tag() << std::endl;
		evaluated[s.tag()] = true;
		return s;
	}

	template<typename Phen>
	void _send(size_t s, size_t current,
			std::vector<boost::shared_ptr<Phen> >& pop)
	{
		dbg::trace t1("mpi", DBG_HERE);
//		using namespace boost::mpi;
		_world->send(s, current, pop[current]->gen());
	}

	template<typename Phen>
	void _slave_loop(const typename Phen::fit_t& fit_proto){
		//TODO: Do something with fit_proto
		dbg::trace t1("mpi", DBG_HERE);
		size_t current_task = 0;

		while(true){
			dbg::out(dbg::info, "mpi") << "Worker " << _world->rank()
					<< " waiting for message in eval_world_seed.hpp"
					<< std::endl;
			boost::mpi::status s = _world->probe();
			dbg::out(dbg::info, "mpi") << "Worker " << _world->rank()
					<< " receveived message in eval_world_seed.hpp tag: "
					<< s.tag() << " source: " << s.source()  << std::endl;
			if (s.tag() == _env->max_tag()){
				_world->recv(0, _env->max_tag(), current_task);
				dbg::out(dbg::info, "mpi") << "Worker " << _world->rank()
						<< " receveived task: " << current_task
						<< " out of maximum tasks: " << NR_OF_MPI_TASKS
						<< std::endl;
				if(current_task == NR_OF_MPI_TASKS){
					dbg::out(dbg::info, "mpi") <<
							"Task exceeded maximum task, interpret as quit." <<
							std::endl;
					MPI_INFO << "[slave] Quit requested" << std::endl;
					MPI_Finalize();
					exit(0);
				}
			}
			else
			{
				switch(current_task){
				case 0:
				{
					boost::fusion::at_c<0>(_tasks).run(_world, s, _env);
					break;
				}
#if NR_OF_MPI_TASKS > 1
				case 1:{
					boost::fusion::at_c<1>(_tasks).run(_world, s, _env);
					break;
				}
#endif
#if NR_OF_MPI_TASKS > 2 
				case 2:{
					boost::fusion::at_c<2>(_tasks).run(_world, s, _env);
					break;
				}
#endif
#if NR_OF_MPI_TASKS > 3 
				case 3:{
					boost::fusion::at_c<3>(_tasks).run(_world, s, _env);
					break;
				}
#endif
				default:
				{
					std::cout << "Unkown task: " << current_task << std::endl;
					MPI_INFO << "[slave] Quit requested" << std::endl;
					MPI_Finalize();
					exit(0);
				}
				}
			}
		}
	}


	long _world_seed;
	env_ptr_t _env;
	com_ptr_t _world;
	Tasks _tasks;
};

}
}

#endif
