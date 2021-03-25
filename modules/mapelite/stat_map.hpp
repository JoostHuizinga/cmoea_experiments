#ifndef STAT_MAP_HPP_
#define STAT_MAP_HPP_

#include <numeric>
#include <fstream>
#include <boost/multi_array.hpp>
#include <sferes/stat/stat.hpp>
#include "map_elite.hpp"

namespace sferes
{
namespace stat
{
SFERES_STAT(Map, Stat)
{
public:
    typedef Phen phen_t;
    typedef boost::shared_ptr<phen_t> indiv_t;
    typedef std::vector<indiv_t> pop_t;
//    typedef boost::multi_array<indiv_t, 2> array_t;
    typedef boost::array<float, 2> point_t;

    //The x resolution of the map
    static const size_t res_x = Params::ea::res_x;

    //The y resolution of the map
    static const size_t res_y = Params::ea::res_y;



    Map(){
        dbg::trace trace("stat", DBG_HERE);
    }


    template<typename E>
    void refresh(const E& ea)
    {
        dbg::trace trace("stat", DBG_HERE);
        dbg::out(dbg::info, "stat") << "Gen: " << ea.gen() << " period: " << Params::stats::period << std::endl;
        if (ea.gen() % Params::stats::period != 0) return;
        pop_t pop = ea.pop();

        dbg::out(dbg::info, "stat") << "Pop size: " << pop.size() << std::endl;

        //Clear all individuals
        size_t best_indiv_index = 0;
        _all.clear();
        for (size_t i = 0; i < pop.size(); ++i){
            if(pop[i]->fit().value() > pop[best_indiv_index]->fit().value()){
                best_indiv_index = i;
            }
            _all.push_back(pop[i]);
        }

        dbg::out(dbg::info, "stat") << "All size: " << _all.size() << std::endl;

        _best_individual = _all[best_indiv_index];

//        if (ea.gen() % Params::pop::dump_period == 0) {
//            std::string fname = ea.res_dir() + "/archive_" + boost::lexical_cast<std::string>(ea.gen()) + ".dat";
//            _write_archive(fname, ea);
//        }
    }


    void log_individual(std::ostream& os, indiv_t indiv){
        dbg::trace trace("stat", DBG_HERE);
        indiv->develop();
        indiv->fit().set_mode(fit::mode::view);
        indiv->fit().eval(*indiv);

        os << indiv->fit().getId();
        os << " " << indiv->fit().value();
        os << " " << indiv->fit().length();
        os << " " << indiv->fit().mod();
        os << " " << indiv->fit().get_regularity_score();
        os << " " << indiv->fit().get_original_length(*indiv);
        os << " " << indiv->fit().get_regularity_score()/indiv->fit().get_original_length(*indiv);
        os << " " << 1.0-(indiv->fit().get_regularity_score()/indiv->fit().get_original_length(*indiv));
        os << "\n";
    }


    void show(std::ostream& os, size_t k)
    {
        dbg::trace trace("stat", DBG_HERE);
        std::cout << "Visualizing for k: " << k << " from: " << _all.size() << std::endl;

        //We are responsible for evaluating networks, so we have to do the work of the slaves
        if(Params::mpi::slaveInit != 0){
            (*Params::mpi::slaveInit)();
        }

        if(options::map["plot-best"].as<bool>()){
            std::cout << "Visualizing best individual." << std::endl;
            if(!_best_individual || options::map["recalc-best"].as<bool>()){
                _best_individual = _calculateBest();
            }

            log_individual(os, _best_individual);
        } else if(k < _all.size()){
            std::cout << "Visualizing individual: " << k << std::endl;
            _all[k]->develop();
            _all[k]->show(os);
            _all[k]->fit().set_mode(fit::mode::usr1);
            _all[k]->fit().eval(*_all[k]);
            _all[k]->fit().visualizeNetwork(*_all[k]);
        } else {
            //TODO: Error message
        }
    }


    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        dbg::trace trace("stat", DBG_HERE);
        ar& BOOST_SERIALIZATION_NVP(_best_individual);
        ar& BOOST_SERIALIZATION_NVP(_all);
    }

    pop_t& getPopulation(){
        dbg::trace trace("stat", DBG_HERE);
        return _all;
    }

    std::string name(){
        return "Map";
    }


    template<typename EA>
    void write_archive(const std::string& fname, const EA& ea) const
    {
        dbg::out(dbg::info, "stat") << "Archive size: " << _all.size() << std::endl;
        std::cout << "writing: " << fname << std::endl;
        std::ofstream ofs(fname.c_str());
        for (size_t i = 0; i<_all.size(); ++i){
            point_t point = ea.get_point(_all[i]);
            float x = point[0];
            float y = point[1];
            ofs << x << " " << y << " " << _all[i]->fit().value() << "\n";
        }
    }

protected:
    indiv_t _best_individual;
    pop_t _all;

    indiv_t _calculateBest(){
        size_t best_index = 0;
        float best_fitness = _all[0]->fit().value();

        for(size_t i=0; i<_all.size(); ++i){
            float fitness = _all[i]->fit().value();
            if(fitness > best_fitness){
                best_fitness = fitness;
                best_index = i;
            }
        }

        return _all[best_index];
    }

//    template<typename EA>
//    void _write_parents(const array_t& array,
//            const array_t& p_array,
//            const std::string& prefix,
//            const EA& ea) const
//    {
//        std::cout << "writing..." << prefix << ea.gen() << std::endl;
//        std::string fname =  ea.res_dir() + "/"
//                + prefix
//                + boost::lexical_cast<
//                std::string>(ea.gen())
//                + std::string(".dat");
//        std::ofstream ofs(fname.c_str());
//        for (size_t i = 0; i < _xs; ++i)
//            for (size_t j = 0; j < _ys; ++j)
//                if (array[i][j] && p_array[i][j])
//                {
//                    point_t p =ea.get_point(p_array[i][j]);
//                    //point_t p = _get_point(p_array[i][j]);
//                    size_t x = round(p[0] * _xs);
//                    size_t y = round(p[1] * _ys);
//                    ofs << i / (float) _xs
//                            << " " << j / (float) _ys
//                            << " " << p_array[i][j]->fit().value()
//                            << " " << x / (float) _xs
//                            << " " << y / (float) _ys
//                            << " " << array[i][j]->fit().value()
//                            << std::endl;
//                }
//    }




};
}
}

#endif
