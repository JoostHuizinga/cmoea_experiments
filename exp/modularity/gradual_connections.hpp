#ifndef GRADUAL_CONNECTIONS_HPP_
#define GRADUAL_CONNECTIONS_HPP_

namespace sferes
{
namespace modif
{
SFERES_CLASS(Gradual){
public:

	// we start at -1 to allow a dump _before_ the switch
	Gradual(){
	}

	template<typename Ea>
	void apply(Ea& ea)
	{
		switch(ea.gen()){
		case 100: Params::ea::pressure = 0.3; break;
		case 200: Params::ea::pressure = 0.4; break;
		case 300: Params::ea::pressure = 0.5; break;
		case 400: Params::ea::pressure = 0.6; break;
		case 500: Params::ea::pressure = 0.7; break;
//		default:
			//Do nothing
		}

#ifdef DIV
          _mod_div.apply(ea);
#endif
	}

#ifdef DIV
        BehaviorDiv<Params> _mod_div;
#endif
};
}
}
#endif //GRADUAL_CONNECTIONS_HPP_
