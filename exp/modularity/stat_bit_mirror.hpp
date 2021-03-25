#ifndef STAT_BIT_MIRROR_HPP_
#define STAT_BIT_MIRROR_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>

namespace sferes
{
  namespace stat
  {
     // lexical order
    struct compare_2val_objs
    {
    	compare_2val_objs(){
      }

      template<typename I>
      bool operator() (const boost::shared_ptr<I> i1, const boost::shared_ptr<I> i2) const
      {
        assert(i1->fit().objs().size() == i2->fit().objs().size());
        assert(i1->fit().objs().size());
        assert(i2->fit().objs().size());
         // first, compare the main objective
        if (i1->fit().value() > i2->fit().value())
          return true;
        if (i1->fit().value() < i2->fit().value())
          return false;

        //Compare them on the second objective
        if (i1->fit().values(1) > i2->fit().values(1))
          return true;
        if (i1->fit().values(1) < i2->fit().values(1))
          return false;

         // equality : compare other objectives
        for (size_t i = 0; i < i1->fit().objs().size(); ++i){
          if (i1->fit().obj(i) > i2->fit().obj(i)){
            return true;
          }else if (i1->fit().obj(i) < i2->fit().obj(i)){
            return false;
          }
        }
        return false;
      }
    };


    SFERES_STAT(BitMirror, Stat)
    {
    	typedef typename Phen::gen_t::nn_t nn_t;
    	typedef typename Phen::gen_t::graph_t graph_t;
        typedef typename nn_t::vertex_desc_t vertex_desc_t;

      public:
        template<typename E>
        void refresh(const E& ea)
        {
          this->_create_log_file(ea, "modularity.dat");
          if (ea.gen() % Params::stats::period != 0) return;
          typedef std::vector<boost::shared_ptr<Phen> > pareto_t;
          typedef typename E::pop_t pop_t;

          pareto_t pareto_front = ea.pareto_front();
          pareto_t pop = ea.pop();
          pop_t mixed_pop = ea.mixed_pop();

          std::sort(pareto_front.begin(), pareto_front.end(), compare_2val_objs());
          std::vector<float> pop_performance_training, pop_performance_test;

          size_t perfect_training = 0;
          size_t perfect_test = 0;
          _all.clear();


          //The current population (after selection)
          for (size_t i = 0; i < pop.size(); ++i){
        	  pop_performance_training.push_back(pop[i]->fit().values(0));
        	  pop_performance_test.push_back(pop[i]->fit().values(1));
        	  if(pop[i]->fit().values(0) >= 1.0) perfect_training++;
        	  if(pop[i]->fit().values(1) >= 1.0) perfect_test++;
          }

          float pop_avg_training = std::accumulate(pop_performance_training.begin(), pop_performance_training.end(), 0.0f) / pop_performance_training.size();
          float pop_avg_test = std::accumulate(pop_performance_test.begin(), pop_performance_test.end(), 0.0f) / pop_performance_test.size();

          //The mixed population (before selection)
          for (size_t i = 0; i < ea.mixed_pop().size(); ++i){
        	  _all.push_back(mixed_pop[i]);
          }

          std::sort(pop_performance_training.begin(), pop_performance_training.end());
          std::sort(pop_performance_test.begin(), pop_performance_test.end());

          (*this->_log_file) << ea.gen() << " "                 // generation
        		  	  	  	 //Best stats
                  	  	  	 << pareto_front.size() << " "				// number of optimal indivs
                             << pareto_front[0]->fit().values(0) << " "  // best fitness (value)
                             << pareto_front[0]->fit().values(1) << " "  // best complete (value)
                             << perfect_training << " "
                             << perfect_test << " "

                             //Pop stats
                             << pop_performance_training.size() << " "	    			// Size of the population
                             << pop_avg_training << " "						// Average fitness of the population
                             << pop_avg_test << " "						// Average modularity of the population
                             << pop_performance_training[pop_performance_training.size() / 2] << " "    	// Median fitness of the population
                             << pop_performance_test[pop_performance_test.size() / 2] << " "    	// Median modularity of the population
                             ;
          (*this->_log_file) << std::endl;

        }

        void show(std::ostream& os, size_t k)
        {
        	std::cout << "Visualizing for k: " << k << " from: " << _all.size() << std::endl;

        	if(k < _all.size()){
        		std::cout << "Visualizing individual: " << k << std::endl;
        		_all[k]->develop();
        		_all[k]->show(os);
        		_all[k]->fit().set_mode(fit::mode::view);
        		_all[k]->fit().eval(*_all[k]);
        	} else {
        		std::cout << "Visualizing all " << _all.size() << " individuals." << std::endl;
        		for(int i=0; i<_all.size(); i++){
        			_all[i]->develop();
            		_all[i]->fit().set_mode(fit::mode::view);
            		std::ostringstream ss;
            		ss << std::setw( 4 ) << std::setfill( '0' ) << i;
            		_all[i]->fit().setPostFix(ss.str());
            		_all[i]->fit().eval(*_all[i]);
            		os << i;
            		os << " " << _all[i]->fit().values(0);
            		os << " " << _all[i]->fit().values(1);
            		os << "\n";
        		}
        	}
        }

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
          ar& BOOST_SERIALIZATION_NVP(_all);
        }

      protected:
        std::vector<boost::shared_ptr<Phen> > _all;
    };
  }
}


#endif //STAT_BIT_MIRROR_HPP_
