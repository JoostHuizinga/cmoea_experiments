#include <iostream>
#include <ssrc/spatial/kd_tree.h>
#include <boost/multi_array.hpp>
namespace sferes
{
  namespace modif
  {
    SFERES_CLASS(Novmap)
    {
      public:
        typedef std::array<float, 2> point_t;
        typedef ssrc::spatial::kd_tree<point_t, int> tree_t;
        typedef tree_t::knn_iterator knn_iterator_t;
        typedef boost::multi_array<float, 2> array_t;

        static const size_t nb_neigh = 8;
        static const size_t res_x = 200;
        static const size_t res_y = 200;

	template<typename Indiv>
	  float cost(const Indiv& ind)
	{    
#if defined(NCONNS)
	    return ind->fit().nb_nodes() / 25.0f;
#elif defined(NNODES)
	    return ind->fit().nb_conns() / 150.0f;
#else
	    return ind->fit().optlength() / 500.0f;      
#endif
	  
	}
        Novmap() :
          _array(boost::extents[res_x][res_y])
        {
          for (size_t i = 0; i < res_x; ++i)
            for (size_t j = 0; j < res_y; ++j)
              _array[i][j] = -1;
        }

        template<typename Ea> void apply(Ea& ea)
        {
          // tree_t apop;
          // for (size_t i = 0; i < ea.pop().size(); ++i)
          // {
          //   point_t p;
          //   p[0] = ea.pop()[i]->fit().mod();

          //   p[1] = cost(ea.pop()[i]);
          //   apop.insert(p, ea.pop()[i]->fit().value());
          // }

          for (size_t i = 0; i < ea.pop().size(); ++i)
          {
             // compute scores
            point_t p;
            p[0] = ea.pop()[i]->fit().mod();
            p[1] = cost(ea.pop()[i]);
            float nov = _mean_distance(p, _archive);
            float fit = ea.pop()[i]->fit().value();
            size_t nb_objs = ea.pop()[i]->fit().objs().size();
            ea.pop()[i]->fit().set_obj(nb_objs - 1, nov);
            ea.pop()[i]->fit().set_obj(0, fit);

             // add the point if the position is free or  if it's better
            size_t x = round(p[0] * res_x);
            size_t y = round(p[1] * res_y);
            assert(x < res_x);
            assert(y < res_y);
            x = std::min(x, res_x);
            y = std::min(y, res_y);
            if (ea.pop()[i]->fit().value() > _array[x][y])
            {
              _array[x][y] = ea.pop()[i]->fit().value(), _array[x][y];
              _archive.insert(p, ea.pop()[i]->fit().value());
            }
          }
          if (ea.gen() % 10 == 0)
          {
            std::cout << "writing..." << ea.gen() << std::endl;
            std::ofstream ofs("archive.dat");
            for (size_t i = 0; i < res_x; ++i)
              for (size_t j = 0; j < res_y; ++j)
                if (_array[i][j] != -1)
                  ofs << i / (float) res_x
                      << " " << j / (float) res_y
                      << " " << _array[i][j] << std::endl;
          }
        }


      protected:
	float _mean_distance(const point_t &p, const tree_t& apop)
	{
	  std::pair<knn_iterator_t, knn_iterator_t>
	    range = apop.find_nearest_neighbors(p, nb_neigh, true);
	  float nov = 0;
	  for (knn_iterator_t it = range.first, end = range.second;
	       it != end; ++it)
            {
              float x = it->first[0] - p[0];
              float y = it->first[1] - p[1];
              nov += sqrtf(x * x + y * y);
            }
            nov /= nb_neigh;
	    return nov;
	}
        tree_t _archive;
        array_t _array;
    };
  }
}
