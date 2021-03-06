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




#define NO_PARALLEL
#ifndef STATIC_BOOST
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE nsga3

// Always enable debugging for our test cases
#define DBG_ENABLED
#undef NDEBUG


#include <boost/test/unit_test.hpp>
#include <cmath>
#include <iostream>
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/stat/pareto_front.hpp>
#include <sferes/eval/parallel.hpp>
#include <sferes/modif/dummy.hpp>

#include <modules/nsga3/nsga3.hpp>
#include <modules/continue/global_options.hpp>

using namespace sferes;
using namespace sferes::gen::evo_float;

struct Params {
	struct evo_float {
		SFERES_CONST float cross_rate = 0.5f;
		SFERES_CONST float mutation_rate = 1.0f / 30.0f;
		SFERES_CONST float eta_m = 15.0f;
		SFERES_CONST float eta_c = 10.0f;
		SFERES_CONST mutation_t mutation_type = polynomial;
		SFERES_CONST cross_over_t cross_over_type = sbx;
	};
	struct pop {
		SFERES_CONST unsigned size = 100;
		SFERES_CONST unsigned select_size = 100;
		SFERES_CONST unsigned nb_gen = 500;
		SFERES_CONST float initial_aleat = 2.0f;
		SFERES_CONST int dump_period = -1;
	};
	struct nsga3 {
		SFERES_CONST unsigned nb_of_objs = 2;
		SFERES_CONST float ref_points_delta = 0.0101010101; // 1/99
	};
	struct parameters {
		SFERES_CONST float min = 0.0f;
		SFERES_CONST float max = 1.0f;
	};
};

template<typename Indiv>
float _g(const Indiv &ind) {
	float g = 0.0f;
	assert(ind.size() == 30);
	for (size_t i = 1; i < 30; ++i)
		g += ind.data(i);
	g = 9.0f * g / 29.0f;
	g += 1.0f;
	return g;
}


SFERES_FITNESS(FitZDT2, sferes::fit::Fitness) {
public:
	template<typename Indiv>
	void eval(Indiv& ind) {
		this->_objs.resize(2);
		float f1 = ind.data(0);
		float g = _g(ind);
		float h = 1.0f - pow((f1 / g), 2.0);
		float f2 = g * h;
		this->_objs[0] = -f1;
		this->_objs[1] = -f2;
	}
};


/**
 * Make available program's arguments to all tests, receiving this fixture.
 *
 * These arguments ensure that debugging statements can be enabled, which is
 * useful for when the test fails.
 */
struct ArgsFixture {
	ArgsFixture(): argc(boost::unit_test::framework::master_test_suite().argc),
			argv(boost::unit_test::framework::master_test_suite().argv){}
	int argc;
	char **argv;
};


BOOST_FIXTURE_TEST_CASE(test_nsga3, ArgsFixture) {
	std::cout << "Testing nsga3..."<<std::endl;
	sferes::options::parse_and_init(argc, argv, false);

	srand(time(0));

	typedef gen::EvoFloat<30, Params> gen_t;
	typedef phen::Parameters<gen_t, FitZDT2<Params>, Params> phen_t;
	typedef eval::Parallel<Params> eval_t;
	typedef boost::fusion::vector<stat::ParetoFront<phen_t, Params> >  stat_t;
	typedef modif::Dummy<> modifier_t;
	typedef ea::nsga3::Nsga3<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;

	ea_t ea;

	// TODO: Should probably write make the code below into a couple of proper tests
//	ea.structured_reference_points().clear();
//	for(int i =0; i<100; ++i){
//		//Eigen::Vector2d target(-0.5, -0.5);
////		Eigen::Vector2d target(-1, 0);
//		target = target.array() + (Eigen::Vector2d::Random()*0.01).array();
//		ea.preferred_reference_points().push_back(target);
//	}
	ea.run();

	ea.stat<0>().show_all(std::cout, 0);
	BOOST_CHECK(ea.stat<0>().pareto_front().size() > 50);

	BOOST_FOREACH(boost::shared_ptr<phen_t> p, ea.stat<0>().pareto_front()) {
		std::cout<<_g(*p)<<std::endl;
		BOOST_CHECK(_g(*p) < 1.1);
		BOOST_CHECK(_g(*p) > 0.0);

	}

}
