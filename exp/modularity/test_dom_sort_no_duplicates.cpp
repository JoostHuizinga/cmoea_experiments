/*
 * test_dom_sort_no_duplicates.cpp
 *
 *  Created on: Oct 19, 2015
 *      Author: Joost Huizinga
 */

#if defined(LAPTOP)
#define NO_PARALLEL
#else
#define TBB
#endif

#define GEN1
#define POP4

#include <modules/continue/global_options.hpp>
#include <modules/misc/params.hpp>
#include <modules/misc/pnsga_params.hpp>
#include <modules/misc/rand.hpp>

#include "dom_sort_no_duplicates.hpp"
#include "pnsga.hpp"

SFERES_PARAMS(
struct Params{
    //Parameters related to the EA (PNSGA in this case)
    struct ea{
        SFERES_ARRAY(float, obj_pressure, 1.0, 0.25,  1.0);
    };
};
)


using namespace sferes;

class TestFit{
public:
    TestFit(float a, float b, float c){
        if(a > -1.0) _objs.push_back(a);
        if(b > -1.0) _objs.push_back(b);
        if(c > -1.0) _objs.push_back(c);
    }

    std::vector<float>& objs(){
        return _objs;
    }

    float obj(int i){
        return _objs[i];
    }

    std::vector<float> _objs;

};

class TestIndiv{
public:
    TestIndiv(float a=-1.0, float b=-1.0, float c=-1.0, std::string name=""):
        _fit(a, b, c), name(name)
    {

    }

    TestFit& fit(){
        return _fit;
    }

    TestFit _fit;
    std::string name;
};

int main(int argc, char **argv) {
    std::cout << "Starting test" << std::endl;

    using namespace options;
    using namespace boost::program_options;
    options::parse_and_init(argc, argv, false);
    set_seed();


    std::vector<TestIndiv*> pop;
    std::vector<std::vector<TestIndiv*> > fronts;
    std::vector<size_t> ranks;

    //Test 1
//    for(size_t i=0; i<1000; ++i){
//        pop.push_back(new TestIndiv(1.0, 1.0, 1.0));
//    }
//
//    std::cout << "Population allocated" << std::endl;
//
//
//    std::cout << "Sorting..." << std::endl;
//    ea::dom_sort_no_duplicates_deb<TestIndiv*, ea::_dom_sort_basic::non_dominated_f>(pop, fronts, ea::_dom_sort_basic::non_dominated_f(), ranks);
//    std::cout << "Sorting... done" << std::endl;
//
//    for(size_t i=0; i<fronts.size(); ++i){
//        std::cout << "Front: " << i << " has size " << fronts[i].size() << std::endl;
//    }
//    for(size_t i=0; i<1000; ++i){
//        delete pop[i];
//    }
//    fronts.clear();
//    ranks.clear();


    //Test 2
//    for(size_t i=0; i<4; ++i){
//        pop.push_back(new TestIndiv(1000.0, 1.0, 1.0));
//    }
//
//    for(size_t i=0; i<4; ++i){
//        pop.push_back(new TestIndiv((float)i, 1.0, 1.0));
//    }
//    std::cout << "Sorting..." << std::endl;
//    ea::dom_sort_no_duplicates_deb<TestIndiv*, ea::_dom_sort_basic::non_dominated_f>(pop, fronts, ea::_dom_sort_basic::non_dominated_f(), ranks);
//    std::cout << "Sorting... done" << std::endl;
//
//    for(size_t i=0; i<fronts.size(); ++i){
//        std::cout << "Front: " << i << " has size " << fronts[i].size() << std::endl;
//    }

    //Test 3
    for(size_t i=0; i<4; ++i){
        pop.push_back(new TestIndiv(1.0, 1.0, 0.5, "champion " + boost::lexical_cast<std::string>(i)));
    }
    std::cout << "Adding 4 champions: 1.0, 1.0, 0.5" << std::endl;

    for(size_t i=0; i<3; ++i){
        pop.push_back(new TestIndiv(1.0, 0.8, 0.5, "second best " + boost::lexical_cast<std::string>(i)));
    }
    std::cout << "Adding 3 second best: 1.0, 0.8, 0.5" << std::endl;

    for(size_t i=0; i<2; ++i){
        pop.push_back(new TestIndiv(1.0, 0.3, 0.5, "third best " + boost::lexical_cast<std::string>(i)));
    }
    std::cout << "Adding 2 third best: 1.0, 0.3, 0.5" << std::endl;

    for(size_t i=0; i<4; ++i){
        pop.push_back(new TestIndiv(0.5, 1.0, 0.5, "loser " + boost::lexical_cast<std::string>(i)));
    }
    std::cout << "Adding 4 losers: 0.5, 1.0, 0.5" << std::endl;

    for(size_t i=0; i<4; ++i){
        pop.push_back(new TestIndiv(0.5, 0.3, 0.5 + float(i)*0.1, "diversity " + boost::lexical_cast<std::string>(i)));
    }
    std::cout << "Adding 4 diversity: 0.5, 0.3, 0.5 + float(i)*0.1" << std::endl;

    std::cout << "Sorting..." << std::endl;
    ea::dom_sort_no_duplicates_deb(pop, fronts, ea::_dom_sort_basic::non_dominated_f(), ranks);
    std::cout << "Sorting... done" << std::endl;

    for(size_t i=0; i<fronts.size(); ++i){
        std::cout << "Front: " << i << " contains ";
        for(size_t j=0; j<fronts[i].size(); ++j){
            std::cout << fronts[i][j]->name << ", ";
        }
        std::cout << std::endl;
    }

    //Test 4
    std::cout << "Sorting..." << std::endl;
    ea::dom_sort_no_duplicates_deb(pop, fronts, ea::pnsga::prob_dom_f<Params>(), ranks);
    std::cout << "Sorting... done" << std::endl;

    for(size_t i=0; i<fronts.size(); ++i){
        std::cout << "Front: " << i << " contains ";
        for(size_t j=0; j<fronts[i].size(); ++j){
            std::cout << fronts[i][j]->name << ", ";
        }
        std::cout << std::endl;
    }





    return 0;
}


