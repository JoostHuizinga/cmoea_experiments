/*
 * nsga3.hpp
 *
 *  Created on: Feb 21, 2019
 *      Author: Joost Huizinga
 */

#ifndef SFERES2_MODULES_NSGA3_NSGA3_HPP_
#define SFERES2_MODULES_NSGA3_NSGA3_HPP_

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>

#include <sferes/stc.hpp>
#include <sferes/parallel.hpp>
#include <sferes/ea/ea.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/ea/dom_sort.hpp>
#include <sferes/ea/common.hpp>
#include <sferes/ea/crowd.hpp>
#include <sferes/ea/nsga2.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <modules/datatools/common_compare.hpp>

namespace sferes {
namespace ea {
namespace nsga3 {


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
 * Calculate the ASF.
 */
inline double calculate_asf(const Eigen::VectorXd& vector, const Eigen::VectorXd& weight){
	double result = vector[0]/weight[0];
	double current;
	for(size_t i=1; i<vector.size(); ++i){
		current = vector[i]/weight[i];
		if(current > result){
			result = current;
		}
	}
	return result;
}

/**
 * Find the ideal point of a population of non-normalized objective vectors.
 */
inline Eigen::VectorXd get_element_wise_max(const std::vector<Eigen::VectorXd>& obj_vectors){
	assert(obj_vectors.size() > 0);
	size_t nb_objs = obj_vectors[0].size();
	Eigen::VectorXd ideal_point(nb_objs);
	for(size_t obj_i=0; obj_i < nb_objs; ++obj_i){
		ideal_point[obj_i] = obj_vectors[0][obj_i];
	}

	for(size_t indiv_i=1; indiv_i<obj_vectors.size(); ++indiv_i){
		for(size_t obj_i=0; obj_i < nb_objs; ++obj_i){
			if(obj_vectors[indiv_i][obj_i] > ideal_point[obj_i]){
				ideal_point[obj_i] = obj_vectors[indiv_i][obj_i];
			}
		}
	}
	return ideal_point;
}

/**
 * Obtain the ideal point from a population of vectors.
 */
inline void invert_and_subtract_ideal(const Eigen::VectorXd ideal_point, std::vector<Eigen::VectorXd>& obj_vectors){
	for(size_t i=0; i<obj_vectors.size(); ++i){
		obj_vectors[i] = ideal_point - obj_vectors[i];
	}
}

/**
 * Obtain the weight vector for every objective.
 */
inline std::vector<Eigen::VectorXd> get_weight_vectors(size_t nb_objs){
	std::vector<Eigen::VectorXd> axis(nb_objs);
	for(size_t i=0; i<nb_objs; ++i){
		axis[i] = Eigen::VectorXd(nb_objs);
		for(size_t obj_i=0; obj_i < nb_objs; ++obj_i){
			if(i == obj_i){
				axis[i][obj_i] = 1;
			} else {
				axis[i][obj_i] = 0.000001;
			}
		}
	}
	return axis;
}

/**
 * Calculate the extreme points out of a population of weight vectors
 */
inline Eigen::MatrixXd get_extreme_points(
		const std::vector<Eigen::VectorXd>& weight_vectors,
		const std::vector<Eigen::VectorXd>& obj_vectors)
{
	assert(obj_vectors.size() > 0);
	size_t nb_objs = obj_vectors[0].size();
	Eigen::MatrixXd extreme_points(nb_objs, nb_objs);
	double min_asf;
	double current_asf;
	for(size_t obj_i=0; obj_i<nb_objs; ++obj_i){
		extreme_points.block(obj_i, 0, 1, nb_objs) = obj_vectors[0].transpose();
		min_asf = calculate_asf(obj_vectors[0], weight_vectors[obj_i]);
		for(size_t i=1; i<obj_vectors.size(); ++i){
			current_asf = calculate_asf(obj_vectors[i], weight_vectors[obj_i]);
			if(current_asf < min_asf){
				min_asf = current_asf;
				extreme_points.block(obj_i, 0, 1, nb_objs) = obj_vectors[i].transpose();
			}
		}
	}
	return extreme_points;
}

/**
 * Calculate the intercept with the different axis
 */
inline Eigen::VectorXd calculate_intercepts(
		const Eigen::MatrixXd& extreme_points,
		const Eigen::VectorXd& backup_intercepts)
{
	Eigen::VectorXd ones = Eigen::VectorXd::Ones(extreme_points.rows());
	// Possibly replace by solve if too slow in practice
	Eigen::MatrixXd inverse = extreme_points.inverse();
	Eigen::VectorXd intercepts = ones.transpose() * inverse;
	intercepts = ones.array() / intercepts.array();

	// The paper says to subtract the ideal point,
	// but that doesn't look correct to me.
	//	intercepts = ideal_point - intercepts;

	// The paper does not indicate what we should do if the extreme
	// points do not actually define a proper hyperplane.

	// Our solution is based on the implementation presented in:
	// Yuan, Yuan, Hua Xu, and Bo Wang.
	// "An experimental investigation of variation operators in reference-point based many-objective optimization."
	// Proceedings of the 2015 Annual Conference on Genetic and Evolutionary Computation. ACM, 2015.
	//
	// As was suggested here: https://www.researchgate.net/post/Is_there_a_fully_functional_NSGA-III_implementation
	if(intercepts.hasNaN()){
//		intercepts = Eigen::VectorXd::Ones(extreme_points.rows());
		intercepts =  backup_intercepts;
	}
	return intercepts;
}

/**
 * Normalize the current objective vectors with respect to the intercepts.
 */
inline void divide_by_intercepts(Eigen::VectorXd intercepts, std::vector<Eigen::VectorXd>& obj_vectors){
	for(size_t i=0; i<intercepts.size(); ++i){
		if(intercepts[i] == 0){
			intercepts[i] = 1.0;
		}
	}
	for(size_t i=0; i<obj_vectors.size(); ++i){
		obj_vectors[i] = obj_vectors[i].array() / intercepts.array();
	}
}

void create_reference_points(
		size_t obj,
		size_t nb_obj,
		double sum,
		double delta,
		Eigen::VectorXd current,
		std::vector<Eigen::VectorXd>& result)
{
	if(obj == (nb_obj-1)){
//		std::cout << "obj: " << obj << std::endl;
		current[nb_obj-1] = 1-sum;
		result.push_back(current);
	} else {
		for(double v=sum, local_v=0; v<=(1.0+delta/2); v+=delta, local_v+=delta){
			Eigen::VectorXd new_current = current;
//			std::cout << "obj: " << obj << " v: " << v << std::endl;
			new_current[obj] = local_v;
			create_reference_points(obj+1, nb_obj, v, delta, new_current, result);
		}
	}
}

std::vector<Eigen::VectorXd> create_reference_points(size_t nb_obj, double delta)
{
	std::vector<Eigen::VectorXd> result;
	Eigen::VectorXd current(nb_obj);
	create_reference_points(0, nb_obj, 0, delta, current, result);
	return result;
}

typedef std::pair<double, size_t> candidate_t;
typedef std::vector<candidate_t> candidates_t;

double calc_distance(
		const Eigen::VectorXd& obj,
		Eigen::VectorXd ref)
{
	ref.normalize();
	return (obj - (ref.transpose() * obj) * ref).norm();
}

candidate_t get_associated_index(
		const Eigen::VectorXd& obj,
		const std::vector<Eigen::VectorXd>& reference_points)
{
	double smallest_distance = std::numeric_limits<double>::infinity();
	double current_distance;
	size_t associated_index = std::numeric_limits<size_t>::max();;
	for(size_t j=0; j<reference_points.size(); ++j){
//		Eigen::VectorXd ref = reference_points[j];
//		ref.normalize();
		current_distance = calc_distance(obj, reference_points[j]);
		if(current_distance < smallest_distance){
			smallest_distance = current_distance;
			associated_index = j;
		}
//		std::cout << "point: " << format_vec(obj) << std::endl;
//		std::cout << "vec: " << format_vec(reference_points[j]) << std::endl;
//		std::cout << "dist: " << current_distance << std::endl;
	}
	dbg::out(dbg::info, "nsga3_select") << "point: " << format_vec(obj) <<
			" index found: " << associated_index <<
			" ref size: " << reference_points.size() <<
			" smallest distance: " << smallest_distance <<
			" current distance: " << current_distance << std::endl;
	dbg::assertion(DBG_ASSERTION(associated_index < reference_points.size()));
	dbg::out(dbg::info, "nsga3_select") << "point: " << format_vec(obj) <<
			" vec: " << format_vec(reference_points[associated_index]) <<
			" dist: " << smallest_distance << std::endl;
	return candidate_t(smallest_distance, associated_index);
}


Eigen::VectorXd project_on_hyperplane(Eigen::VectorXd& point){
	Eigen::VectorXd offset(point.size());
	offset.fill(1.0/double(point.size()));
	Eigen::VectorXd point_on_plane = -1.0*point + offset;
	Eigen::VectorXd norm = offset.normalized();
	double distance = point_on_plane.transpose() * norm;
	Eigen::VectorXd projection = point + norm * distance;
	return projection;
}

void project_on_hyperplane(std::vector<Eigen::VectorXd>& reference_points)
{
	for(size_t i=0; i<reference_points.size(); ++i){
		reference_points[i] = project_on_hyperplane(reference_points[i]);
	}
}


std::vector<size_t> select_from_candidates(
		size_t to_be_selected,
		std::vector<size_t> associated_count,
		std::vector<candidates_t> associated_candidates)
{
	dbg::out(dbg::info, "ea") << "NSGA3: To be selected: " << to_be_selected << std::endl;
	dbg::out(dbg::info, "ea") << "NSGA3: Associated counts: " << associated_count.size() << std::endl;
	dbg::out(dbg::info, "ea") << "NSGA3: Candidates: " << associated_candidates.size() << std::endl;

	std::vector<size_t> selected_objs;
	std::vector<std::pair<size_t, size_t> > associated_count_sorted;
	for(size_t i=0; i<associated_count.size(); ++i){
		associated_count_sorted.push_back(std::pair<size_t, size_t>(associated_count[i], i));
	}
	compare::sort(associated_count_sorted);

	// Calculates the max index from which to select (exclusive)
	size_t max_index=0;
	while(associated_count_sorted[0].first == associated_count_sorted[max_index].first and max_index < associated_count_sorted.size()){
		max_index += 1;
	}

	while(to_be_selected > 0){
		dbg::out(dbg::info, "nsga3_select") << "NSGA3: Max index: " << max_index << std::endl;
		int sorted_index = misc::randInt(max_index);
		size_t selected = associated_count_sorted[sorted_index].second;
		dbg::out(dbg::info, "nsga3_select") << "NSGA3: Selected: " << selected << std::endl;

		if(associated_candidates[selected].size() > 0){
			dbg::out(dbg::info, "nsga3_select") << "NSGA3: Candidates available: " << associated_candidates[selected].size() << std::endl;
			candidates_t candidates = associated_candidates[selected];
			candidate_t chosen;
			size_t chosen_index;
			if(associated_count[selected] == 0){
				dbg::out(dbg::info, "nsga3_select") << "NSGA3: No points already associated; choosing closest" << std::endl;
				chosen = candidate_t(std::numeric_limits<double>::max(), 0);
				for(size_t i=0; i<candidates.size(); ++i){
					if(candidates[i].first < chosen.first){
						chosen = candidates[i];
						chosen_index = i;
					}
				}
			} else {
				dbg::out(dbg::info, "nsga3_select") << "NSGA3: Points already associated "
						<< associated_candidates[selected].size() << "; selecting random" << std::endl;
				chosen_index = misc::randInt(candidates.size());
				chosen = candidates[chosen_index];
			}
			selected_objs.push_back(chosen.second);
			associated_candidates[selected].erase(associated_candidates[selected].begin() + chosen_index);
			associated_count[selected] += 1;
			to_be_selected -= 1;
			// Fix associated sorted
			if(max_index > 1){
				max_index -= 1;
				std::pair<size_t, size_t> temp = associated_count_sorted[sorted_index];
				associated_count_sorted[sorted_index] = associated_count_sorted[max_index];
				associated_count_sorted[max_index] = temp;
			} else {
				while(associated_count_sorted[0].first == associated_count_sorted[max_index].first and max_index < associated_count_sorted.size()){
					max_index += 1;
				}
			}

		} else {
			dbg::out(dbg::info, "nsga3_select") << "NSGA3: No candidates available" << std::endl;
			associated_count[selected] = std::numeric_limits<size_t>::max();
			associated_count_sorted.erase(associated_count_sorted.begin() + sorted_index);

			// Fix associated sorted
			if(max_index > 1){
				max_index -= 1;
			} else {
				while(associated_count_sorted[0].first == associated_count_sorted[max_index].first and max_index < associated_count_sorted.size()){
					max_index += 1;
				}
			}
		}
	}
	return selected_objs;
}

std::vector<size_t> get_associated_count(
		const std::vector<Eigen::VectorXd>& objs,
		const std::vector<Eigen::VectorXd>& reference_points)
{
	candidate_t associated_index;
	std::vector<size_t> associated_count(reference_points.size(), 0);

	for(size_t i=0; i<objs.size(); ++i){
		associated_index = get_associated_index(objs[i], reference_points);
		associated_count[associated_index.second] += 1;
	}
	return associated_count;
}

std::vector<candidates_t> get_associated_candidates(
		const std::vector<Eigen::VectorXd>& objs,
		const std::vector<Eigen::VectorXd>& reference_points)
{
	candidate_t associated_index;
	std::vector<candidates_t> associated_candidates(reference_points.size());

	for(size_t i=0; i<objs.size(); ++i){
		associated_index = get_associated_index(objs[i], reference_points);
		size_t ref_index = associated_index.second;
		associated_index.second = i;
		associated_candidates[ref_index].push_back(associated_index);
	}
	return associated_candidates;
}




template<typename Phen, typename Eval, typename Stat, typename FitModifier, typename Params,
typename Exact = stc::Itself>
class Nsga3 : public Ea <Phen, Eval, Stat, FitModifier, Params,
typename stc::FindExact<Nsga3<Phen, Eval, Stat, FitModifier, Params, Exact>, Exact>::ret > {
public:
	typedef boost::shared_ptr<Phen> indiv_t;
	typedef typename std::vector<indiv_t> pop_t;
	typedef typename pop_t::iterator it_t;
	typedef typename std::vector<std::vector<indiv_t> > front_t;
	typedef Ea <Phen, Eval, Stat, FitModifier, Params,
			typename stc::FindExact<Nsga3<Phen, Eval, Stat, FitModifier, Params, Exact>, Exact>::ret > parent_t;
	SFERES_EA_FRIEND(Nsga3);

	Nsga3(): parent_t()
	{
		dbg::trace trace("ea", DBG_HERE);
		_weight_vectors = nsga3::get_weight_vectors(Params::nsga3::nb_of_objs);
		_structured_reference_points = nsga3::create_reference_points(Params::nsga3::nb_of_objs, Params::nsga3::ref_points_delta);

		// Print debug information if requested
		dbg::out(dbg::info, "ea") << "NSGA3: Weight vector size: " << _weight_vectors.size() << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Number of objectives: " << Params::nsga3::nb_of_objs << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Number of reference points: " << _structured_reference_points.size() << std::endl;
		dbg::assertion(DBG_ASSERTION(_weight_vectors.size() > 0));
		dbg::assertion(DBG_ASSERTION(_structured_reference_points.size() > 0));
		dbg::assertion(DBG_ASSERTION(_weight_vectors.size() == Params::nsga3::nb_of_objs));
	}

	void random_pop()
	{
		dbg::trace trace("ea", DBG_HERE);
		parallel::init();
		this->_pop.resize(Params::pop::size);
		dbg::assertion(DBG_ASSERTION(Params::pop::select_size % 4 == 0));
		pop_t init_pop((size_t)(Params::pop::size * Params::pop::initial_aleat));
		parallel::p_for(parallel::range_t(0, init_pop.size()), random<Phen>(init_pop));
		_eval_subpop(init_pop);
		_apply_modifier(init_pop); // Note: _apply_modifier overwrites this->_pop
		_fill_nondominated_sort(init_pop, this->_pop);
	}

	// a step
	void epoch()
	{
		dbg::trace trace("ea", DBG_HERE);
		_mixed_pop.clear();
		_child_pop.clear();

		_selection (this->_pop, _child_pop, Params::pop::select_size);
		parallel::p_for(parallel::range_t(0, _child_pop.size()), mutate<Phen>(_child_pop));
#ifndef EA_EVAL_ALL
		_eval_subpop(_child_pop);
		compare::merge(this->_pop, _child_pop, _mixed_pop);
#else
		compare::merge(this->_pop, _child_pop, _mixed_pop);
		_eval_subpop(_mixed_pop);
#endif
		_apply_modifier(_mixed_pop); // Note: _apply_modifier overwrites this->_pop
#ifndef NDEBUG
		BOOST_FOREACH(indiv_t& ind, _mixed_pop){
			for (size_t i = 0; i < ind->fit().objs().size(); ++i) {
				dbg::assertion(DBG_ASSERTION(!std::isnan(ind->fit().objs()[i])));
			}
		}
#endif
		_fill_nondominated_sort(_mixed_pop, this->_pop);
		dbg::assertion(DBG_ASSERTION(this->_pop.size() == Params::pop::size));
	}
	const pop_t& pareto_front() const {
	  return _pareto_front;
	}
	const pop_t& child_pop() const {
		return _child_pop;
	}
	const pop_t& mixed_pop() const {
		return _mixed_pop;
	}
	const pop_t& parent_pop() const {
		return this->_pop;
	}
	std::vector<Eigen::VectorXd>& weight_vectors() {
		return _weight_vectors;
	}
	std::vector<Eigen::VectorXd>& structured_reference_points() {
		return _structured_reference_points;
	}
	std::vector<Eigen::VectorXd>& preferred_reference_points() {
		return _preferred_reference_points;
	}
	const std::vector<Eigen::VectorXd>& weight_vectors() const{
		return _weight_vectors;
	}
	const std::vector<Eigen::VectorXd>& structured_reference_points() const{
		return _structured_reference_points;
	}
	const std::vector<Eigen::VectorXd>& preferred_reference_points() const{
		return _preferred_reference_points;
	}
	const std::vector<size_t>& associated_count() const{
		return _associated_count;
	}
	const std::vector<nsga3::candidates_t>& associated_candidates() const{
		return _associated_candidates;
	}
	const Eigen::VectorXd& intercepts() const{
		return _intercepts;
	}

protected:
	pop_t _pareto_front;
	pop_t _child_pop;
	pop_t _mixed_pop;
	std::vector<Eigen::VectorXd> _weight_vectors;
	std::vector<Eigen::VectorXd> _structured_reference_points;
	std::vector<Eigen::VectorXd> _preferred_reference_points;
	std::vector<size_t> _associated_count;
	std::vector<nsga3::candidates_t> _associated_candidates;
	Eigen::VectorXd _intercepts;

	void _eval_subpop(pop_t& pop)
	{
		dbg::trace trace("ea", DBG_HERE);
		this->_eval_pop(pop, 0, pop.size());
	}

	void _apply_modifier(pop_t& pop)
	{
		dbg::trace trace("ea", DBG_HERE);
		this->_pop.clear();
		this->_pop.insert(this->_pop.end(), pop.begin(), pop.end());
		this->apply_modifier();
	}

	void _fill_nondominated_sort(const pop_t& mixed_pop, pop_t& new_pop)
	{
		dbg::trace trace("ea", DBG_HERE);
		dbg::assertion(DBG_ASSERTION(mixed_pop.size()));
		dbg::assertion(DBG_ASSERTION(Params::nsga3::nb_of_objs == mixed_pop[0]->fit().objs().size()));
		front_t fronts;
		std::vector<size_t> ranks;
		pop_t proposed_new_pop;

#ifndef NDEBUG
		BOOST_FOREACH(const indiv_t& ind, mixed_pop){
			for (size_t i = 0; i < ind->fit().objs().size(); ++i) {
				dbg::assertion(DBG_ASSERTION(!std::isnan(ind->fit().objs()[i])));
			}
		}
#endif

		// Sort the population into fronts
		dbg::out(dbg::info, "ea") << "NSGA3: Sorting population" << std::endl;
		dom_sort(mixed_pop, fronts, ranks);

		// Keep track of the Pareto front (not otherwise used)
		dbg::out(dbg::info, "ea") << "NSGA3: Storing Pareto front" << std::endl;
		_pareto_front.clear();
		_pareto_front.insert(_pareto_front.end(), fronts[0].begin(), fronts[0].end());

		// Fill the i first layers
		dbg::out(dbg::info, "ea") << "NSGA3: Create proposed population" << std::endl;
		size_t selected = 0;
		for (size_t i = 0; i < fronts.size(); ++i){
			proposed_new_pop.insert(proposed_new_pop.end(), fronts[i].begin(), fronts[i].end());
			if (proposed_new_pop.size() < Params::pop::size){
				selected += fronts[i].size();
			} else {
				break;
			}
		}

		size_t to_select = Params::pop::size - selected;

		// Print debug information if requested
		dbg::out(dbg::info, "ea") << "NSGA3: Perform selection" << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Proposed size: " << proposed_new_pop.size() << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Population size: " << Params::pop::size << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Selected size: " << selected << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: To select size: " << to_select << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Weight vector size: " << _weight_vectors.size() << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Number of objectives: " << Params::nsga3::nb_of_objs << std::endl;
		dbg::out(dbg::info, "ea") << "NSGA3: Number of reference points: " << _structured_reference_points.size() << std::endl;
		dbg::assertion(DBG_ASSERTION(proposed_new_pop.size() >= Params::pop::size));
		dbg::assertion(DBG_ASSERTION(selected + to_select == Params::pop::size));
		dbg::assertion(DBG_ASSERTION(_weight_vectors.size() == Params::nsga3::nb_of_objs));

		new_pop = _select_nsga3(proposed_new_pop,
				selected,
				to_select,
				_weight_vectors,
				_structured_reference_points,
				_preferred_reference_points);

 		dbg::assertion(DBG_ASSERTION(new_pop.size() == Params::pop::size));
	}

	template<typename Indiv>
	std::vector<Indiv> _select_nsga3(
			const std::vector<Indiv>& proposed_new_pop,
			size_t selected,
			size_t to_select,
			const std::vector<Eigen::VectorXd>& weight_vectors,
			const std::vector<Eigen::VectorXd>& structured_reference_points,
			const std::vector<Eigen::VectorXd>& prefered_reference_points)
	{
		typedef Eigen::VectorXd objs_t;
		assert(proposed_new_pop.size() > 0);
		std::vector<Indiv> new_pop;
		size_t nb_objs = proposed_new_pop[0]->fit().objs().size();

		// Fill the first part of the new population
		dbg::out(dbg::info, "ea") << "NSGA3: Filling first i of new pop" << std::endl;
		new_pop.assign(proposed_new_pop.begin(), proposed_new_pop.begin() + selected);

		// Extract the objective values from a population of individuals
		dbg::out(dbg::info, "ea") << "NSGA3: Extracting objective values from pop" << std::endl;
		std::vector<Eigen::VectorXd> normalized_objs(proposed_new_pop.size());
		for(size_t i=0; i<normalized_objs.size(); ++i){
			normalized_objs[i] = Eigen::VectorXd (nb_objs);
			for(size_t obj_i=0; obj_i < nb_objs; ++obj_i){
				normalized_objs[i][obj_i] = proposed_new_pop[i]->fit().objs()[obj_i];
			}
		}

		// Initialize the ideal point
		dbg::out(dbg::info, "ea") << "NSGA3: Initializing ideal point" << std::endl;
		Eigen::VectorXd ideal_point = get_element_wise_max(normalized_objs);

		// Normalize such that the ideal point lies at (0, 0, 0)
		// Note: from this point one, we are minimizing the normalized scores
		dbg::out(dbg::info, "ea") << "NSGA3: Subtracting ideal point" << std::endl;
		invert_and_subtract_ideal(ideal_point, normalized_objs);
#if defined(ROBUST_INTERCEPTS)
		dbg::out(dbg::info, "ea") << "NSGA3: Get robust intercepts" << std::endl;
		_intercepts = get_element_wise_max(normalized_objs);
#else
		dbg::out(dbg::info, "ea") << "NSGA3: Get back-up intercepts" << std::endl;
		Eigen::VectorXd backup_intercepts = get_element_wise_max(normalized_objs);

		// Calculate extreme points
		dbg::out(dbg::info, "ea") << "NSGA3: Calculating extreme points" << std::endl;
		Eigen::MatrixXd extreme_points = get_extreme_points(weight_vectors, normalized_objs);

		// Calculate intercepts
		dbg::out(dbg::info, "ea") << "NSGA3: Calculating intercepts" << std::endl;
		_intercepts = calculate_intercepts(extreme_points, backup_intercepts);
#endif
		// Finish normalizing objectives
		dbg::out(dbg::info, "ea") << "NSGA3: Normalizing objectives" << std::endl;
		divide_by_intercepts(_intercepts, normalized_objs);

		// Calculate reference points
		std::vector<Eigen::VectorXd> reference_points;
		reference_points.insert(reference_points.end(), structured_reference_points.begin(), structured_reference_points.end());

		std::vector<Eigen::VectorXd> prefered_reference_points_copy = prefered_reference_points;
		invert_and_subtract_ideal(ideal_point, prefered_reference_points_copy);
		divide_by_intercepts(_intercepts, prefered_reference_points_copy);
		project_on_hyperplane(prefered_reference_points_copy);
		reference_points.insert(reference_points.end(), prefered_reference_points_copy.begin(), prefered_reference_points_copy.end());

		// Test associated count
		dbg::out(dbg::info, "ea") << "NSGA3: Calculating associate counts" << std::endl;
		std::vector<Eigen::VectorXd> selected_objs;
		selected_objs.assign(normalized_objs.begin(), normalized_objs.begin() + selected);
		_associated_count = nsga3::get_associated_count(selected_objs, reference_points);

		// Test associate candidates
		dbg::out(dbg::info, "ea") << "NSGA3: Gathering candidates" << std::endl;
		std::vector<Eigen::VectorXd> candidate_objs;
		candidate_objs.assign(normalized_objs.begin() + selected, normalized_objs.end());
		_associated_candidates = nsga3::get_associated_candidates(candidate_objs, reference_points);

		// Select from candidates
		dbg::out(dbg::info, "ea") << "NSGA3: Selecting candidates" << std::endl;
		std::vector<size_t> selected_indices = nsga3::select_from_candidates(to_select, _associated_count, _associated_candidates);

		dbg::out(dbg::info, "ea") << "NSGA3: Adding selected candidates to new population" << std::endl;
		for(size_t i=0; i<selected_indices.size(); ++i){
			new_pop.push_back(proposed_new_pop[selected + selected_indices[i]]);
		}

		return new_pop;
	}

	// --- tournament selection ---
	void _selection(const pop_t& old_pop, pop_t& new_pop, size_t select_size)
	{
		dbg::trace trace("ea", DBG_HERE);
		new_pop.resize(select_size);
		std::vector<size_t> a1, a2;
		misc::rand_ind(a1, old_pop.size());
		misc::rand_ind(a2, old_pop.size());
		// todo : this loop could be parallelized
		for (size_t i = 0; i < select_size; i += 4)
		{
			const indiv_t& p1 = _tournament(old_pop[a1[i]], old_pop[a1[i + 1]]);
			const indiv_t& p2 = _tournament(old_pop[a1[i + 2]], old_pop[a1[i + 3]]);
			const indiv_t& p3 = _tournament(old_pop[a2[i]], old_pop[a2[i + 1]]);
			const indiv_t& p4 = _tournament(old_pop[a2[i + 2]], old_pop[a2[i + 3]]);
			assert(i + 3 < new_pop.size());
			p1->cross(p2, new_pop[i], new_pop[i + 1]);
			p3->cross(p4, new_pop[i + 2], new_pop[i + 3]);
		}
	}

	const indiv_t& _tournament(const indiv_t& i1, const indiv_t& i2)
	{
		dbg::trace trace("ea", DBG_HERE);
		int flag = fit::dominate_flag(i1, i2);
		if (flag == 1){
			return i1;
		}
		if (flag == -1){
			return i2;
		}
		// TODO: This operation should probably make use of how crowded the associated reference line is
		// such that parents from sparsely populated lines have a higher chance of being selected.
		if (misc::flip_coin()){
			return i1;
		} else {
			return i2;
		}
	}
};
}
}
}

#endif /* SFERES2_MODULES_NSGA3_NSGA3_HPP_ */
