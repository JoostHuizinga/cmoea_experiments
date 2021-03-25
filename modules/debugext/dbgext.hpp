#ifndef DBGEXT_DBG_H
#define DBGEXT_DBG_H
#include <sferes/dbg/dbg.hpp>
#include <modules/continue/global_options.hpp>
#include <iostream>

namespace dbgext
{

#ifdef DBG_ENABLED
inline std::ostream &dbo(dbg::dbg_source src) {
  return dbg::out(dbg::info, src);
}
#else
inline dbg::null_stream dbo(dbg::dbg_source src)         {
    return dbg::null_stream();
}
#endif

template <class _CharT, class _Traits>
inline std::basic_ostream<_CharT, _Traits>&
dbe(std::basic_ostream<_CharT, _Traits>& __os)
{
    __os.put(__os.widen('\n'));
    __os.flush();
    return __os;
}

template<typename Type>
std::string str(Type number){
	return boost::lexical_cast<std::string>(number);
}

}

namespace dbg
{

#ifdef DBG_ENABLED

bool enabled(level lvl, std::string src) {
  bool result = false;
  if(sferes::options::map.count("verbose")){
    std::vector<std::string> streams = sferes::options::map["verbose"].as<std::vector<std::string> >();
    result = std::find(streams.begin(), streams.end(), src) != streams.end();
  }
  return result;
}

#define DBG_CONDITIONAL(LEVEL, SOURCE, FUNCTION) \
  if(dbg::enabled(LEVEL, SOURCE)){               \
      FUNCTION;                                  \
  }

#else

    /**************************************************************************
     * Non-debug stub versions
     *************************************************************************/

    /*
     * With debugging switched off we generate null versions of the above
     * definitions.
     *
     * Given a good compiler and a strong prevailing headwind, these will
     * optimise away to nothing.
     */

bool enabled(level lvl, std::string src) { return false;}
#define DBG_CONDITIONAL(LEVEL, SOURCE, FUNCTION)

#endif
}

#endif
