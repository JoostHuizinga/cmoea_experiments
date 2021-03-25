/*
 * patterns.hpp
 *
 *  Created on: Sep 24, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_PATTERNS_HPP_
#define MODULES_MISC_PATTERNS_HPP_

//Standard includes
#include <cmath>
#include <set>
#include <bitset>

//Sferes includes
#include <sferes/dbg/dbg.hpp>

/********************************
 *          PATTERNS            *
 ********************************/

/**
 * Add a pattern to be classified to a problem.
 */
template<typename Params>
void addPattern(unsigned problemIndex, bool i1, bool i2, bool i3, bool i4){
    dbg::assertion(DBG_ASSERTION(Params::problems::layers(0)==4));
    dbg::assertion(DBG_ASSERTION(problemIndex < Params::problems::nb_of_problems));
    unsigned problemMod = problemIndex;
    unsigned base = (i1?1:0) + (i2?2:0) + (i3?4:0) + (i4?8:0);
    unsigned patterns_per_problem = 1 << Params::problems::layers(0);
    unsigned skip = std::pow(patterns_per_problem, problemMod+1);
    unsigned block = std::pow(patterns_per_problem, problemMod);
    unsigned offset = block * base;
    dbg::out(dbg::info, "patterns") << "Problem: " << problemIndex
                                    << " has pattern: " << i1 << i2 << i3 << i4
                                    << " at index: " << base << std::endl;
    for(unsigned i=offset; i<Params::problems::nb_of_patterns ; i += skip){
        for(unsigned j=0; j<block; ++j){
            dbg::out(dbg::info, "patterns") << "- index set: " << i+j << std::endl;
            Params::problems::answers_set(problemIndex, i + j, true);
        }
    }
}

/**
 * Removes a pattern to be classified.
 */
template<typename Params>
void removePattern(unsigned problemIndex, bool i1, bool i2, bool i3, bool i4){
    dbg::assertion(DBG_ASSERTION(Params::problems::layers(0)==4));
    dbg::assertion(DBG_ASSERTION(problemIndex < Params::problems::nb_of_problems));
    unsigned problemMod = problemIndex;
    unsigned base = (i1?1:0) + (i2?2:0) + (i3?4:0) + (i4?8:0);
    unsigned patterns_per_problem = 1 << Params::problems::layers(0);
    unsigned skip = std::pow(patterns_per_problem, problemMod+1);
    unsigned block = std::pow(patterns_per_problem, problemMod);
    unsigned offset = block * base;
    dbg::out(dbg::info, "patterns") << "Problem: " << problemIndex
                                    << " has pattern: " << i1 << i2 << i3 << i4
                                    << " at index: " << base << std::endl;
    for(unsigned i=offset; i<Params::problems::nb_of_patterns ; i += skip){
        for(unsigned j=0; j<block; ++j){
            dbg::out(dbg::info, "patterns") << "- index set: " << i+j << std::endl;
            Params::problems::answers_set(problemIndex, i + j, false);
        }
    }
}


/**
 * Checks whether a certain answer is a pattern or not.
 */
template<typename Params>
inline bool checkAnswer(const unsigned& pattern, const unsigned& problem){
    dbg::out(dbg::info, "patterns") << "- Problem: " << problem << " max problems: " << Params::problems::nb_of_problems << std::endl;
    dbg::out(dbg::info, "patterns") << "  - Pattern: " << pattern << " max pattern: " << Params::problems::nb_of_patterns << std::endl;
    dbg::assertion(DBG_ASSERTION(pattern < Params::problems::nb_of_patterns));
    dbg::assertion(DBG_ASSERTION(problem < Params::problems::nb_of_problems));
    return Params::problems::answers(problem, pattern);
}

/**
 * Prints all patterns that where added.
 */
template<typename Params>
void printPatterns(){
    std::set<unsigned long> input_sets_seen;
    for(unsigned problem=0; problem<Params::problems::nb_of_problems; ++problem){
        std::cout << "Patterns problem: " << problem << std::endl;
        input_sets_seen.clear();
        for(unsigned i=0; i<Params::problems::nb_of_patterns; ++i){
            std::bitset<Params::problems::nb_of_bits> pattern(i);
            std::bitset<Params::problems::inputs_per_problem> problem_pattern;
            int start = Params::problems::layers(0) * problem;
            int end = Params::problems::layers(0) * (problem+1);
            unsigned target_bit = Params::problems::layers(0)-1;
            for(int source_bit = end-1; source_bit >= start; --source_bit){
                problem_pattern[target_bit] = pattern[source_bit];
                --target_bit;
            }
            unsigned long problem_pattern_l = problem_pattern.to_ulong();
            if(checkAnswer<Params>(i, problem) && input_sets_seen.count(problem_pattern_l) == 0){
                std::cout << " - ";
                for (size_t j = 0; j < problem_pattern.size(); ++j){
                    std::cout << (problem_pattern[j] ? 1 : 0);
                }
                std::cout << std::endl;
                input_sets_seen.insert(problem_pattern_l);
            }
        }
    }
}



#endif /* MODULES_MISC_PATTERNS_HPP_ */
