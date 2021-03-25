/*
 * test_nsga3.cpp
 *
 *  Created on: Feb 22, 2019
 *      Author: Joost Huizinga
 *
 * Because the boost test suite is the first to parse parameters, to enable
 * debug output, you should call this test case as follows:
 * build/modules/cmoea/test_nsga3 -- --verbose nsga3
 */
// Defines
#ifndef STATIC_BOOST
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE test_nsga3_components

// Always enable debugging for our test cases
#define DBG_ENABLED
#undef NDEBUG

// Boost includes
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

// Module includes
#include <modules/continue/global_options.hpp>
#include <modules/datatools/clock.hpp>

// Local includes
#include "nsga3.hpp"

// Local debug macro
#define DBO dbg::out(dbg::info, "nsga3")

// Name spaces
using namespace sferes;
using namespace sferes::ea;


/**
 * Convenience function for formatting Eigen vectors horizontally.
 */
std::string format_vec(Eigen::VectorXd vec){
    std::ostringstream ss;
    ss << std::setprecision(5);

	std::string result;
	for(size_t i=0; i<vec.size(); ++i){
		ss << vec[i] << " ";
//		result += boost::lexical_cast<std::string>(vec[i]) + " ";
	}
	return ss.str();
}

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


BOOST_FIXTURE_TEST_CASE(test_nsga3_components, ArgsFixture){
	std::cout << "Test nsga3_test started." << std::endl;
    sferes::options::parse_and_init(argc, argv, false);
//    DBO << std::setprecision(5) << std::endl;

    // Test asf
    Eigen::VectorXd test1(4);
    Eigen::VectorXd test2(4);
    test1 << 1, 2, 3, 4;
    test2 << 0.000001, 0.000001, 0.000001, 1;
    DBO << "ASF 1:" << nsga3::calculate_asf(test1, test2) << std::endl;
    test1 << 0.000001, 0.000001, 0.000001, 4;
    test2 << 0.000001, 0.000001, 0.000001, 1;
    DBO << "ASF 2:" << nsga3::calculate_asf(test1, test2) << std::endl;

    // Set parameters
	const double reference_point_delta = 0.5;
	const size_t nb_objectives = 3;
	const size_t nb_individuals = 11;

    // Create population of objectives
    std::vector<Eigen::VectorXd> objs;
    objs.push_back(Eigen::Vector3d( 0.5,  0.3,  0.4)); // Indiv 1: On the Pareto front being best at obj 0
    objs.push_back(Eigen::Vector3d( 0.2,  0.7,  0.0)); // Indiv x: Best on obj 1
    objs.push_back(Eigen::Vector3d(-0.1,  0.5,  0.7)); // Indiv 2: On the Pareto front being best at obj 1 and 2
    objs.push_back(Eigen::Vector3d( 0.4,  0.6, -0.2)); // Indiv 3: On the Pareto front with best trade-off between obj 0 and obj 1
    objs.push_back(Eigen::Vector3d( 0.1,  0.4,  0.5)); // Indiv x: Best trade-off an objs 1 and 2
    objs.push_back(Eigen::Vector3d( 0.4, -0.1,  0.5)); // Indiv x: Best trade-off an objs 0 and 2
    objs.push_back(Eigen::Vector3d( 0.4,  0.4,  0.4)); // Indiv x: Best overall trade-off
    objs.push_back(Eigen::Vector3d( 0.2,  0.1,  0.0)); // Indiv 4: Dominated by indiv 1
    objs.push_back(Eigen::Vector3d(-0.2,  0.3, -0.1)); // Indiv 5: Dominated by indiv 1 and indiv 2
    objs.push_back(Eigen::Vector3d( 0.1,  0.1,  0.1)); //
    objs.push_back(Eigen::Vector3d( 0.0,  0.0,  0.2)); //


    // Test ideal point
    Eigen::VectorXd ideal_point = nsga3::get_element_wise_max(objs);
    DBO << "Ideal point: " << format_vec(ideal_point) << std::endl;
    Eigen::VectorXd expected_ideal_point = Eigen::Vector3d(0.5,  0.7,  0.7);
    DBO << "Expected ideal point: " << format_vec(expected_ideal_point) << std::endl;
    BOOST_TEST(ideal_point == expected_ideal_point);

    // Test normalization with respect to the ideal point
    nsga3::invert_and_subtract_ideal(ideal_point, objs);

    std::vector<Eigen::VectorXd> expected_norm_objs;
    expected_norm_objs.push_back(Eigen::Vector3d( 0.0,  0.4,  0.3)); // 400000, 300000, 400000
    expected_norm_objs.push_back(Eigen::Vector3d( 0.3,  0.0,  0.7)); // 700000, 700000, 300000
    expected_norm_objs.push_back(Eigen::Vector3d( 0.6,  0.2,  0.0)); // 200000, 600000, 600000 Extreme point axis 1
    expected_norm_objs.push_back(Eigen::Vector3d( 0.1,  0.1,  0.9)); // 900000, 900000, 100000 Extreme point axis 3
    expected_norm_objs.push_back(Eigen::Vector3d( 0.4,  0.3,  0.2)); // 300000, 400000, 400000
    expected_norm_objs.push_back(Eigen::Vector3d( 0.1,  0.8,  0.2)); // 800000, 200000, 800000 Extreme point axis 2
    expected_norm_objs.push_back(Eigen::Vector3d( 0.1,  0.3,  0.3)); // 300000, 300000, 300000
    expected_norm_objs.push_back(Eigen::Vector3d( 0.3,  0.6,  0.7)); // 700000, 700000, 600000
    expected_norm_objs.push_back(Eigen::Vector3d( 0.7,  0.4,  0.8)); // 800000, 800000, 700000
    expected_norm_objs.push_back(Eigen::Vector3d( 0.4,  0.6,  0.6)); // 600000, 600000, 600000
    expected_norm_objs.push_back(Eigen::Vector3d( 0.5,  0.7,  0.5)); // 700000, 500000, 700000


    for(size_t i=0; i<objs.size(); ++i){
    	DBO << "Norm obj " << i << ": " << format_vec(objs[i]) << std::endl;
    }
    for(size_t i=0; i<expected_norm_objs.size(); ++i){
    	DBO << "Expected norm obj " << i << ": " << format_vec(expected_norm_objs[i]) << std::endl;
    	for(size_t j=0; j<objs[i].size(); ++j){
    		BOOST_CHECK_CLOSE(objs[i][j], expected_norm_objs[i][j], 0.001);
    	}
    }


//    CHECK_CLOSE_COLLECTION(objs, expected_norm_objs, 0.001);


    // Test weight vectors
    std::vector<Eigen::VectorXd> weight_vectors = nsga3::get_weight_vectors(3);
    for(size_t i=0; i<weight_vectors.size(); ++i){
    	DBO << "Weight vector " << i << ": " << format_vec(weight_vectors[i]) << std::endl;
    }

    std::vector<Eigen::VectorXd> expected_weight_vectors;
    expected_weight_vectors.push_back(Eigen::Vector3d(1.0,  0.000001,  0.000001));
    expected_weight_vectors.push_back(Eigen::Vector3d(0.000001,  1.0,  0.000001));
    expected_weight_vectors.push_back(Eigen::Vector3d(0.000001,  0.000001,  1.0));
    for(size_t i=0; i<expected_weight_vectors.size(); ++i){
    	DBO << "Expected weight vector " << i << ": " << format_vec(expected_weight_vectors[i]) << std::endl;
    }
    BOOST_TEST(std::is_permutation(weight_vectors.begin(), weight_vectors.end(), expected_weight_vectors.begin()));


    // Test ASF
    std::vector<std::vector<double> > expected_asf;
    std::vector<double> v1;
    std::vector<double> v2;
    std::vector<double> v3;
    std::vector<double> v4;
    std::vector<double> v5;
    std::vector<double> v6;
    std::vector<double> v7;
    std::vector<double> v8;
    std::vector<double> v9;
	std::vector<double> v10;
	std::vector<double> v11;
    v1.push_back(400000);
    v1.push_back(300000);
    v1.push_back(400000);
    v2.push_back(700000);
    v2.push_back(700000);
    v2.push_back(300000);
    v3.push_back(200000);
    v3.push_back(600000);
    v3.push_back(600000);
    v4.push_back(900000);
    v4.push_back(900000);
    v4.push_back(100000);
    v5.push_back(300000);
    v5.push_back(400000);
    v5.push_back(400000);
    v6.push_back(800000);
    v6.push_back(200000);
    v6.push_back(800000);
    v7.push_back(300000);
    v7.push_back(300000);
    v7.push_back(300000);
    v8.push_back(700000);
    v8.push_back(700000);
    v8.push_back(600000);
    v9.push_back(800000);
    v9.push_back(800000);
    v9.push_back(700000);
    v10.push_back(600000);
    v10.push_back(600000);
    v10.push_back(600000);
    v11.push_back(700000);
    v11.push_back(500000);
    v11.push_back(700000);
    expected_asf.push_back(v1);
    expected_asf.push_back(v2);
    expected_asf.push_back(v3);
    expected_asf.push_back(v4);
    expected_asf.push_back(v5);
    expected_asf.push_back(v6);
    expected_asf.push_back(v7);
    expected_asf.push_back(v8);
    expected_asf.push_back(v9);
    expected_asf.push_back(v10);
    expected_asf.push_back(v11);
    for(size_t i=0; i<expected_weight_vectors.size(); ++i){
    	for(size_t j=0; j<objs.size(); ++j){
    		double asf = nsga3::calculate_asf(objs[j], expected_weight_vectors[i]);
    		DBO << "ASF " << i << ", "<< j << ": " << asf << std::endl;
    		DBO << "Expected ASF " << i << ", "<< j << ": " << expected_asf[j][i] << std::endl;
    		BOOST_CHECK_CLOSE(asf, expected_asf[j][i], 0.001);
    	}
    }

    // Test extreme points
    Eigen::MatrixXd extreme_points = nsga3::get_extreme_points(weight_vectors, objs);
    DBO << "Extreme points:\n" << extreme_points << std::endl;
    Eigen::MatrixXd expected_extreme_points(nb_objectives, nb_objectives);
    expected_extreme_points <<
    		 0.6,  0.2,  0.0,
			 0.1,  0.8,  0.2,
			 0.1,  0.1,  0.9;
    DBO << "Expected extreme points:\n" << expected_extreme_points << std::endl;
    for(size_t i=0; i<extreme_points.rows(); ++i){
    	for(size_t j=0; j<extreme_points.cols(); ++j){
    		BOOST_CHECK_CLOSE(extreme_points(i,j), expected_extreme_points(i,j), 0.001);
    	}
    }

    // Test intercepts
    Eigen::VectorXd backup_intercepts = nsga3::get_element_wise_max(weight_vectors);
    Eigen::VectorXd intercepts = nsga3::calculate_intercepts(extreme_points, backup_intercepts);
    DBO << "Intercepts:\n" << intercepts << std::endl;
    Eigen::VectorXd expected_intercepts(nb_objectives);
    expected_intercepts << 0.725, 1.26875, 1.06842; // TODO: I have no clue if these numbers are actually correct
    for(size_t i=0; i<expected_intercepts.size(); ++i){
    	BOOST_CHECK_CLOSE(intercepts[i], expected_intercepts[i], 0.001);
    }

    // Test normalization
    nsga3::divide_by_intercepts(intercepts, objs);
    for(size_t i=0; i<objs.size(); ++i){
    	DBO << "Norm obj " << i << ": " << format_vec(objs[i]) << std::endl;
    }
    // TODO: Write actual test
    std::vector<Eigen::VectorXd> expected_real_norm_objs;
    expected_real_norm_objs.push_back(Eigen::Vector3d(0,       0.31527,  0.28079 )); // Associates with 0.0, 0.5, 0.5
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.41379, 0,        0.65517 )); // Associates with 0.5, 0.0, 0.5
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.82759, 0.15764,  0       )); // Associates with 1.0, 0.0, 0.0
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.13793, 0.078818, 0.84236 )); // Associates with 0.0, 0.0, 1.0
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.55172, 0.23645,  0.18719 )); // Associates with 0.5, 0.5, 0.0
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.13793, 0.63054,  0.18719 )); // Associates with 0.0, 1.0, 0.0
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.13793, 0.23645,  0.28079 )); // Associates with 0.0, 0.5, 0.5

    expected_real_norm_objs.push_back(Eigen::Vector3d(0.41379, 0.47291,  0.65517 )); // Associates with 0.0, 0.5, 0.5
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.96552, 0.31527,  0.74877 )); // Associates with 0.5, 0.0, 0.5
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.55172, 0.47291,  0.56158 )); // Associates with 0.5, 0.0, 0.5
    expected_real_norm_objs.push_back(Eigen::Vector3d(0.68966, 0.55172,  0.46798 )); // Associates with 0.5, 0.5, 0.0

    for(size_t i=0; i<expected_real_norm_objs.size(); ++i){
    	DBO << "Expected real norm obj " << i << ": " << format_vec(expected_norm_objs[i]) << std::endl;
    	for(size_t j=0; j<objs[i].size(); ++j){
    		BOOST_CHECK_CLOSE(objs[i][j], expected_real_norm_objs[i][j], 0.01);
    	}
    }

    // Test reference points
	std::vector<Eigen::VectorXd> reference_points = nsga3::create_reference_points(nb_objectives, reference_point_delta);
	for(size_t i=0; i<reference_points.size(); ++i){
		DBO << "Reference point " << i << ": ";
		for(size_t j=0; j<reference_points[i].size(); ++j){
			DBO << reference_points[i][j] << " ";
		}
		DBO << std::endl;
	}
	std::vector<Eigen::VectorXd> expected_reference_points;
	expected_reference_points.push_back(Eigen::Vector3d(1.0, 0.0, 0.0));
	expected_reference_points.push_back(Eigen::Vector3d(0.0, 1.0, 0.0));
	expected_reference_points.push_back(Eigen::Vector3d(0.0, 0.0, 1.0));
	expected_reference_points.push_back(Eigen::Vector3d(0.5, 0.5, 0.0));
	expected_reference_points.push_back(Eigen::Vector3d(0.5, 0.0, 0.5));
	expected_reference_points.push_back(Eigen::Vector3d(0.0, 0.5, 0.5));
	for(size_t i=0; i<expected_reference_points.size(); ++i){
		DBO << "Expected reference point " << i << ": ";
		for(size_t j=0; j<expected_reference_points[i].size(); ++j){
			DBO << expected_reference_points[i][j] << " ";
		}
		DBO << std::endl;
	}
	BOOST_TEST(std::is_permutation(reference_points.begin(), reference_points.end(), expected_reference_points.begin()));


	// Test prefered reference point
	std::vector<Eigen::VectorXd> prefered_reference_points;
	prefered_reference_points.push_back(Eigen::Vector3d(1.0, 1.0, 1.0));
	prefered_reference_points.push_back(Eigen::Vector3d(0.2, 0.7, 0.1));

	nsga3::invert_and_subtract_ideal(ideal_point, prefered_reference_points);
	nsga3::divide_by_intercepts(intercepts, prefered_reference_points);
	nsga3::project_on_hyperplane(prefered_reference_points);

	for(size_t i=0; i<prefered_reference_points.size(); ++i){
		DBO << "Preferred ref " << i << ": " << format_vec(prefered_reference_points[i]) << std::endl;
	}

	{
		Eigen::VectorXd offset(2);
		offset.fill(0.5);
		Eigen::VectorXd point(2);
		point << 0, -3.0;
		Eigen::VectorXd point_on_plane = -1.0*point + offset;
		DBO << "point_on_plane: " << format_vec(point_on_plane) << std::endl;
		Eigen::VectorXd norm = offset.normalized();
		DBO << "norm: " << format_vec(norm) << std::endl;
		double distance = point_on_plane.transpose() * norm;
		DBO << "distance: " << distance << std::endl;
		Eigen::VectorXd projection = point + norm * distance;
		DBO << "projection: " << format_vec(projection) << std::endl;
	}
//	bool permutation = std::is_permutation(answer.begin(), answer.end(), reference_points.begin());


	{
		std::vector<Eigen::VectorXd> reference_points = nsga3::create_reference_points(2, 0.07142857142);
		DBO << "Number of reference points:" << reference_points.size() << std::endl;
		BOOST_CHECK_EQUAL(reference_points.size(), 15);
	}

	{
		std::vector<Eigen::VectorXd> reference_points = nsga3::create_reference_points(6, 1.0/12.0);
		DBO << "Number of reference points:" << reference_points.size() << std::endl;
//		for(size_t i=0; i<reference_points.size(); ++i){
//			std::cout << "Reference point " << i << ": ";
//			for(size_t j=0; j<reference_points[i].size(); ++j){
//				std::cout << reference_points[i][j] << " ";
//			}
//			std::cout << std::endl;
//		}
		BOOST_CHECK_EQUAL(reference_points.size(), 6188);
	}

	// Test distance
	for(size_t j=0; j<objs.size(); ++j){
		for(size_t i=0; i<reference_points.size(); ++i){
			double dist = nsga3::calc_distance(objs[j], reference_points[i]);
			DBO << "Obj: " << format_vec(objs[j]) << " ref: " << format_vec(reference_points[i]) << " dist: " << dist << std::endl;
		}
	}

	// Test selecting 2 individuals
	size_t to_select = 9;
	size_t selected = 7;
	std::vector<Eigen::VectorXd> selected_objs;
	selected_objs.assign(objs.begin(), objs.begin() + selected);
	std::vector<size_t> associated_count = nsga3::get_associated_count(selected_objs, reference_points);

	std::vector<size_t> expected_associated_count;
	expected_associated_count.push_back(1);
	expected_associated_count.push_back(2);
	expected_associated_count.push_back(1);
	expected_associated_count.push_back(1);
	expected_associated_count.push_back(1);
	expected_associated_count.push_back(1);
    for(size_t i=0; i<associated_count.size(); ++i){
    	DBO << "Associated count " << i << " (" << format_vec(reference_points[i]) << "): " << associated_count[i] << std::endl;
    	DBO << "Expected associated count: " << expected_associated_count[i] << std::endl;
    	BOOST_CHECK_EQUAL(associated_count[i], expected_associated_count[i]);
    }

	// Test associate candidates
	std::vector<Eigen::VectorXd> candidate_objs;
	candidate_objs.assign(objs.begin() + selected, objs.end());
	std::vector<nsga3::candidates_t> associated_candidates = nsga3::get_associated_candidates(candidate_objs, reference_points);

    for(size_t i=0; i<candidate_objs.size(); ++i){
    	DBO << "Candidates: " << i << ": " << format_vec(candidate_objs[i]) << std::endl;
    }

	// Test selection
	std::vector<size_t> selected_indices = nsga3::select_from_candidates(to_select - selected, associated_count, associated_candidates);
	std::vector<size_t> expected_selected_indices;
	expected_selected_indices.push_back(3);
	expected_selected_indices.push_back(1);

    for(size_t i=0; i<selected_indices.size(); ++i){
    	DBO << "Selected: " << i << ": " << selected_indices[i] << std::endl;
    }
    for(size_t i=0; i<expected_selected_indices.size(); ++i){
    	DBO << "Expected selected: " << i << ": " << expected_selected_indices[i] << std::endl;
    }
    BOOST_TEST(std::is_permutation(selected_indices.begin(), selected_indices.end(), expected_selected_indices.begin()));

    for(size_t i=0; i<associated_count.size(); ++i){
    	DBO << "Associated count: " << i << ": " << associated_count[i] << std::endl;
    }


    {
    	// Test selecting 6 individuals
    	size_t to_select = 6;
    	size_t selected = 0;
    	std::vector<Eigen::VectorXd> selected_objs;
    	selected_objs.assign(objs.begin(), objs.begin() + selected);
    	std::vector<size_t> associated_count = nsga3::get_associated_count(selected_objs, reference_points);

    	// Test associate candidates
    	std::vector<Eigen::VectorXd> candidate_objs;
    	candidate_objs.assign(objs.begin() + selected, objs.end());
    	std::vector<nsga3::candidates_t> associated_candidates = nsga3::get_associated_candidates(candidate_objs, reference_points);

    	// Test selection
    	std::vector<size_t> selected_indices = nsga3::select_from_candidates(to_select - selected, associated_count, associated_candidates);
        for(size_t i=0; i<selected_indices.size(); ++i){
        	DBO << "Selected: " << i << ": " << selected_indices[i] << std::endl;
        }
    }

    {
    	// Test selecting 10 individuals
    	size_t to_select = 10;
    	size_t selected = 0;
    	std::vector<Eigen::VectorXd> selected_objs;
    	selected_objs.assign(objs.begin(), objs.begin() + selected);
    	std::vector<size_t> associated_count = nsga3::get_associated_count(selected_objs, reference_points);

    	// Test associate candidates
    	std::vector<Eigen::VectorXd> candidate_objs;
    	candidate_objs.assign(objs.begin() + selected, objs.end());
    	std::vector<nsga3::candidates_t> associated_candidates = nsga3::get_associated_candidates(candidate_objs, reference_points);

    	// Test selection
    	std::vector<size_t> selected_indices = nsga3::select_from_candidates(to_select - selected, associated_count, associated_candidates);
        for(size_t i=0; i<selected_indices.size(); ++i){
        	DBO << "Selected: " << i << ": " << selected_indices[i] << std::endl;
        }
    }

    // Test with a more objectives and a larger population size to get an idea of the practical computational complexity of NSGA-III
    {
    	const double reference_point_delta = 0.5;
    	const size_t nb_objectives = 100;
    	const size_t nb_individuals = 1000;

        std::vector<Eigen::VectorXd> objs;
        for (int i=0; i<nb_individuals; ++i){
        	objs.push_back(Eigen::VectorXd::Random(nb_objectives));
        }

        // Test ideal point
        Clock precompute_timer;
        precompute_timer.resetAndStart();
        Eigen::VectorXd ideal_point = nsga3::get_element_wise_max(objs);

        nsga3::invert_and_subtract_ideal(ideal_point, objs);

        std::vector<Eigen::VectorXd> weight_vectors = nsga3::get_weight_vectors(nb_objectives);

        Eigen::MatrixXd extreme_points = nsga3::get_extreme_points(weight_vectors, objs);

        // Test intercepts
        Eigen::VectorXd backup_intercepts = nsga3::get_element_wise_max(weight_vectors);
        Eigen::VectorXd intercepts = nsga3::calculate_intercepts(extreme_points, backup_intercepts);

        // Test reference points
		std::vector<Eigen::VectorXd> reference_points = nsga3::create_reference_points(nb_objectives, reference_point_delta);
		precompute_timer.stop();
		DBO << selected_objs.size() << " precomputation in " << precompute_timer.time() << " ms" << std::endl;

		size_t to_select = nb_individuals/2;
		size_t selected = nb_individuals/4;
		std::vector<Eigen::VectorXd> selected_objs;
		selected_objs.assign(objs.begin(), objs.begin() + selected);
		Clock associated_timer;
		associated_timer.resetAndStart();
		// These are the slow operations
		std::vector<size_t> associated_count = nsga3::get_associated_count(selected_objs, reference_points);
		associated_timer.stop();
		DBO << selected_objs.size() << " objectives associated in " << associated_timer.time() << " ms" << std::endl;

		// Test associate candidates
		std::vector<Eigen::VectorXd> candidate_objs;
		candidate_objs.assign(objs.begin() + selected, objs.end());
		Clock candidates_timer;
		candidates_timer.resetAndStart();
		// These are the slow operations
		std::vector<nsga3::candidates_t> associated_candidates = nsga3::get_associated_candidates(candidate_objs, reference_points);
		candidates_timer.stop();
		DBO << candidate_objs.size() << " candidates gathered in " << candidates_timer.time() << " ms" << std::endl;

		// Test selection
		Clock select_timer;
		select_timer.resetAndStart();
		std::vector<size_t> selected_indices = nsga3::select_from_candidates(to_select - selected, associated_count, associated_candidates);
		select_timer.stop();
		DBO << selected_indices.size() << " candidates selected in " << select_timer.time() << " ms" << std::endl;
//	    for(size_t i=0; i<selected_indices.size(); ++i){
//	    	DBO << "Selected: " << i << ": " << selected_indices[i] << std::endl;
//	    }
    }


}

#undef DBO
