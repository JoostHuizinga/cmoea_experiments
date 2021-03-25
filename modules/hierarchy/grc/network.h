/*
 Header for the network class
 Author: Enys Mones, 2013
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "stdlib.h"
#include "node.h"
#include "file_handler.h"
#include <map>
#include <string>
#include "math.h"
#include "mother.h"
#include "time.h"

void shuffle_vector( std::vector<double> &vec );

class NETWORK
{
private:
    int Order;
    int Size;
    std::vector<NODE*> Nodes;
    CRandomMother Ran;
public:
    typedef std::map<std::pair<std::string , std::string>, float> nn_input_t;
    typedef std::map<std::string, std::pair<double,int> > coord_map_t;;
    typedef std::vector<double> reals_t;

    // constructors
    NETWORK();
    NETWORK( const NETWORK *_other );
    ~NETWORK();

    // structure
    int order() const;
    int size() const;
    void add_node();
    void add_node( const char *_label );
    void remove_node( int _id );
    bool is_there_edge( int _source, int _target ) const;
    void add_edge( int _source, int _target, double _weight );
    void remove_edge( int _source, int _target );
    double get_edge_weight( int _source, int _target ) const;
    void generate_random_graph( int _order, int _size, reals_t &_weights );
    void load_network( std::string& _input_file, bool _weighted,
    		std::string& _output, int _size_limit );
    void load_network2(nn_input_t network, bool _weighted, int _size_limit );
   void clear_network(); 
   void rewire( int _laps );

    // calculations
    void get_weights(reals_t &_weights ) const;
    void get_default_local_reaching_centrality(reals_t& _lrc ) const;
    void get_weighted_local_reaching_centrality(reals_t& _lrc ) const;
    void get_global_reaching_centrality(reals_t& _lrc, float& _grc ) const;
    void get_distribution(reals_t& _lrc, reals_t& _dist ) const;
    void get_coordinates( double _z, double _dx, double _dy,
    		reals_t& _lrc, reals_t& _x, reals_t& _y ) const;
    void get_neuron_coords(reals_t _x, reals_t _y, coord_map_t& neuron_coords);

    // output
    void print_local_reaching_centrality(reals_t &_lrc) const;
    void print_distribution(reals_t &_dist ) const;
    void print_coordinates(reals_t &_x, reals_t &_y,
    		std::string &_output_name) const;
    void print_image(reals_t& _x, reals_t& _y, std::string &_output_name) const;
};


#endif // NETWORK_H
