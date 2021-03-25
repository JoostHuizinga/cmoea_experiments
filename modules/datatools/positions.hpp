/*
 * positions.hpp
 *
 *  Created on: Sep 24, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_POSITIONS_HPP_
#define MODULES_MISC_POSITIONS_HPP_

//Boost include
#include <boost/spirit/include/karma.hpp>

//Sferes include
#include <sferes/dbg/dbg.hpp>

/********************************
 *          POSITIONS           *
 ********************************/
template<typename Params>
inline void setPos(const float& x, const float& y, const float& z,
        const float& x_vis, const float& y_vis, const float& z_vis, size_t& neuron_index){
    Params::dnn::spatial::_x[neuron_index] = x;
    Params::dnn::spatial::_y[neuron_index] = y;
    Params::dnn::spatial::_z[neuron_index] = z;
    Params::visualisation::_x[neuron_index] = x_vis;
    Params::visualisation::_y[neuron_index] = -y_vis;
    Params::visualisation::_z[neuron_index] = z_vis;
    dbg::out(dbg::info, "neuronpos") << "index:" << neuron_index <<
            " x: " << Params::dnn::spatial::_x[neuron_index] <<
            " y: " << Params::dnn::spatial::_y[neuron_index] <<
            " z: " << Params::dnn::spatial::_z[neuron_index] << std::endl;
    ++neuron_index;
}

template<typename Params>
inline void setPos(const float& x, const float& y, const float& z, size_t& neuron_index){
    setPos<Params>(x, y, z, x, y, z, neuron_index);
}

template<typename Params>
inline void setPos(const float& x, const float& z, size_t& neuron_index){
    setPos<Params>(x, 0, z, x, 0, z, neuron_index);
}

template<typename Params>
inline void setPos(const float& x, const float& z, const float& x_vis, const float& z_vis, size_t& neuron_index){
    setPos<Params>(x, 0, z, x_vis, 0, z_vis, neuron_index);
}

template<typename Params>
void print_positions(){
    //Print positions
    float tolerance = 0.00001;
    std::vector<float> x_values;
    for(unsigned i=0; i<Params::dnn::spatial::x_size(); ++i){
        x_values.push_back(Params::dnn::spatial::x(i));
    }
    std::vector<float> y_values;
    for(unsigned i=0; i<Params::dnn::spatial::y_size(); ++i){
        y_values.push_back(Params::dnn::spatial::y(i));
    }

    Range x_range = get_range(x_values, tolerance);
    Range y_range = get_range(y_values, tolerance);

//    std::cout << "x_range: " << x_range << std::endl;
//    std::cout << "y_range: " << y_range << std::endl;

    unsigned nb_x_bins = x_range.nb_of_bins(tolerance);
    unsigned nb_y_bins = y_range.nb_of_bins(tolerance);

//    std::cout << "Number of x bins: " << nb_x_bins << std::endl;
//    std::cout << "Number of y bins: " << nb_y_bins << std::endl;
    std::vector<std::vector<std::vector<unsigned> > > bins(nb_x_bins);
    for(unsigned i=0; i<bins.size(); ++i){
        bins[i].resize(nb_y_bins);
    }

    for(unsigned i=0; i<Params::dnn::spatial::x_size(); ++i){
        unsigned x_bin_index = x_range.get_bin(Params::dnn::spatial::x(i), tolerance);
        unsigned y_bin_index = y_range.get_bin(Params::dnn::spatial::y(i), tolerance);
//        std::cout << "- point " << i << " placed in bin: " << x_bin_index << "," << y_bin_index << std::endl;
        dbg::assertion(DBG_ASSERTION(x_bin_index < nb_x_bins));
        dbg::assertion(DBG_ASSERTION(y_bin_index < nb_y_bins));
        bins[x_bin_index][y_bin_index].push_back(i);
    }

    //Do the actual printing
    using namespace boost::spirit::karma;
    std::cout << "     |";
    for(float x=x_range.min; x<=x_range.max + tolerance; x+=x_range.step){
        std::cout << format(right_align(5, ' ')[maxwidth(5)[float_]], x)  << "|";
    }
    std::cout << std::endl;

    for(int y=nb_y_bins-1; y>=0; --y){
        float y_value = y_range.min +  (y_range.step * y);
        std::cout << format(right_align(5, ' ')[maxwidth(5)[float_]], y_value)  << "|";
        for(unsigned x=0; x<nb_x_bins; ++x){
            dbg::assertion(DBG_ASSERTION(bins[x][y].size() < 2));
            if(bins[x][y].size()==1){
                unsigned neuron_id = bins[x][y][0];
                std::cout << format(right_align(5, ' ')[maxwidth(5)[int_]], neuron_id)  << "|";
            } else {
                std::cout << "     |";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}



#endif /* MODULES_MISC_POSITIONS_HPP_ */
