/*
 * modif_mvg.hpp
 *
 *  Created on: Oct 9, 2015
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_MODIF_MVG_HPP_
#define EXP_MODULARITY_MODIF_MVG_HPP_

#include <sferes/stc.hpp>
#include <sferes/dbg/dbg.hpp>

#include <modules/misc/patterns.hpp>
#include <modules/misc/params.hpp>

SFERES_VALUE_PARAM(MvgPeriod,
        #if defined(MVG1)
            static const size_t v = 1;
        #define MVG_PERIOD_DEFINED
        #endif

        #if defined(MVG10)
            static const size_t v = 10;
        #define MVG_PERIOD_DEFINED
        #endif

        #if defined(MVG1000)
            static const size_t v = 1000;
        #define MVG_PERIOD_DEFINED
        #endif

        #if defined(MVG5000)
            static const size_t v = 5000;
        #define MVG_PERIOD_DEFINED
        #endif

        #if not defined(MVG_PERIOD_DEFINED)
            static const size_t v = 1;
        #endif
)

#ifdef MVG_PERIOD_DEFINED
#define MVG
#endif

namespace sferes{
namespace modif{

SFERES_CLASS(ModifMvg){
public:
    template<typename Ea>
    void apply(Ea& ea)
    {
        dbg::trace trace("modif", DBG_HERE);

        if ((ea.gen() > 0) && ((ea.gen() % Params::mvg::period) == 0)){
            Ea::phen_t::fit_t::next_obj();
            ea.eval().eval(ea.pop(), 0, ea.pop().size());
        }

//        //std::cout << ea.gen() + 1 + Params::mvg::period << " mod " << Params::mvg::period*2 << " = " << ((ea.gen() + 1 + Params::mvg::period) % (Params::mvg::period*2)) << std::endl;
//        if (((ea.gen() + 1 + Params::mvg::period) % (Params::mvg::period*2)) == 0){
////            std::cout << "Switch to new" << std::endl;
//            //Problem 2: Patterns with 3 false and 1 true
//            removePattern<Params>(1, true,  false, false, false);
//            removePattern<Params>(1, false, true,  false, false);
//            removePattern<Params>(1, false, false, true,  false);
//            removePattern<Params>(1, false, false, false, true);
//
//            //Problem 2: Patterns with all false, all true, and 2 false, 2 true
//            addPattern<Params>(1, true,  true, true, true);
//            addPattern<Params>(1, false, false, false, false);
//            addPattern<Params>(1, true, false, false, true);
//            addPattern<Params>(1, false, true, true, false);
//
////            printPatterns<Params>();
//
//        } else if (((ea.gen() + 1) % (Params::mvg::period*2)) == 0){
////            std::cout << "Switch back" << std::endl;
//            //Problem 2: Patterns with 3 false and 1 true
//            addPattern<Params>(1, true,  false, false, false);
//            addPattern<Params>(1, false, true,  false, false);
//            addPattern<Params>(1, false, false, true,  false);
//            addPattern<Params>(1, false, false, false, true);
//
//            //Problem 2: Patterns with all false, all true, and 2 false, 2 true
//            removePattern<Params>(1, true,  true, true, true);
//            removePattern<Params>(1, false, false, false, false);
//            removePattern<Params>(1, true, false, false, true);
//            removePattern<Params>(1, false, true, true, false);
//
////            printPatterns<Params>();
//            ea.eval().eval(ea.pop(), 0, ea.pop().size());
//        }
    }
};
}
}



#endif /* EXP_MODULARITY_MODIF_MVG_HPP_ */
