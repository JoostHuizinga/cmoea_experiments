/*
 * neuron_group.hpp
 *
 *  Created on: Mar 23, 2017
 *      Author: joost
 */

#ifndef EXP_MODULARITY_NEURON_GROUP_HPP_
#define EXP_MODULARITY_NEURON_GROUP_HPP_

// Boost includes
#include <boost/spirit/include/karma.hpp>

// Sferes includes
#include <sferes/dbg/dbg.hpp>

// Module includes
#include <modules/datatools/params.hpp>

class NeuronGroup{
public:
    NeuronGroup(size_t size, bool is_input = false, size_t offset = 0, std::string prefix = "" , size_t local_index = 0):
        _size(size),
        _offset(offset),
        _is_input(is_input)
    {
        _name = prefix + boost::lexical_cast<std::string>(local_index);
    }
    size_t size(){return _size;}
    size_t offset(){return _offset;}
    bool is_input(){return _is_input;}
    std::string name(){return _name;}
private:
    size_t _size;
    size_t _offset;
    bool _is_input;
    std::string _name;
};

class ConnectionGroup{
public:
    ConnectionGroup(size_t source, size_t target, size_t offset = 0):
        _source(source),
        _target(target),
        _offset(offset)
    {}
    size_t source(){return _source;}
    size_t target(){return _target;}
    size_t offset(){return _offset;}
private:
    size_t _source;
    size_t _target;
    size_t _offset;
};

template<typename Params>
class NeuronGroups{
public:
    enum CppnIndex{
        staticIndex,
        sourceIndex,
        targetIndex,
        combinatorialIndex
    };

    enum Layers{
        inputLayer = 0,
        hiddenLayer,
        outputLayer
    };


    NeuronGroups():
        _separateOutputsPerLayer(true),
        _separateOutputsPerLayerNeurons(true),
        _hiddenCppnIndex(sourceIndex),
        _outputCppnIndex(sourceIndex),
        _inputToHiddenCppnIndex(sourceIndex),
        _hiddenToHiddenCppnIndex(staticIndex),
        _hiddenToOutputCppnIndex(targetIndex),
        _nb_cppn_outputs(0)
    {}

    /**
     * Adds an input group of size size_of_neuron_group.
     *
     * Substrate input groups are not associated with a CPPN neuron output group,
     * because substrate inputs do not require bias or time-constant information.
     */
    void addInput(size_t size_of_neuron_group){
        dbg::out(dbg::info, "neuron_groups") << "Input neuron group added of size: " << size_of_neuron_group << std::endl;
        _inputLayer.push_back(NeuronGroup(size_of_neuron_group, true, 0, "in", _inputLayer.size()));
    }

    /**
     * Adds a hidden group of size size_of_neuron_group.
     *
     * If the _hiddenCppnIndex is 'static', all substrate hidden groups will be assigned
     * to the same CPPN neuron output group. Otherwise every substrate hidden group
     * will be assigned to a new CPPN neuron output group.
     *
     * After adding all hidden groups, _nb_cppn_neuron_groups_assigned_to_hidden_layer will reflect
     * the number of CPPN neuron groups currently assigned to neuron groups in the hidden layer.
     */
    void addHidden(size_t size_of_neuron_group){
        dbg::out(dbg::info, "neuron_groups") << "Hidden neuron group added of size: " << size_of_neuron_group << std::endl;
        std::string key = getCppnOutputGroupKey(_hiddenCppnIndex, hiddenLayer, _hiddenLayer.size());
        _hiddenLayer.push_back(NeuronGroup(size_of_neuron_group, false, assign(key), "hid", _hiddenLayer.size()));
    }

    /**
     * Adds an output group of size size_of_neuron_group.
     *
     * If the _outputCppnIndex is 'static', all substrate output groups will be assigned
     * to the same CPPN neuron output group. If _separateOutputsPerLayerNeurons is false,
     * the CPPN neuron output group will be 0. If _separateOutputsPerLayerNeurons is true,
     * the CPPN neuron output group will be _nb_cppn_neuron_groups_assigned_to_hidden_layer.
     *
     */
    void addOutput(size_t size_of_neuron_group){
        dbg::out(dbg::info, "neuron_groups") << "Output neuron group added of size: " << size_of_neuron_group << std::endl;
        std::string key = getCppnOutputGroupKey(_outputCppnIndex, outputLayer, _outputLayer.size());
        _outputLayer.push_back(NeuronGroup(size_of_neuron_group, false, assign(key), "out", _outputLayer.size()));
    }

    size_t getNbInputGroups() const{ return _inputLayer.size();}
    size_t getNbHiddenGroups() const{ return _hiddenLayer.size();}
    size_t getNbOutputGroups() const{ return _outputLayer.size();}

//    size_t getNbCppnNeuronOutputGroupsHidden() const{
//        if(getNbHiddenGroups() == 0) return 0;
//        return _hiddenCppnIndex == staticIndex ? 1 : getNbHiddenGroups();
//    }
//
//    size_t getNbCppnNeuronOutputGroupsOutput() const{
//        if(getNbOutputGroups() == 0) return 0;
//        return _outputCppnIndex == staticIndex ? 1 : getNbOutputGroups();
//    }
//
//    size_t getNbCppnNeuronOutputGroups() const {
//        size_t result;
//        if(_separateOutputsPerLayerNeurons){
//            result = getNbCppnNeuronOutputGroupsHidden() + getNbCppnNeuronOutputGroupsOutput();
//        } else {
//            result = std::max(getNbCppnNeuronOutputGroupsHidden(), getNbCppnNeuronOutputGroupsOutput());
//        }
//        return result;
//    }

    void connectInputToHidden(size_t input_index, size_t hidden_index){
        std::string key = getCppnOutputGroupKey(_inputToHiddenCppnIndex, inputLayer, input_index, hidden_index);
        size_t real_hidden_index = hidden_index + getNbInputGroups();
        _connection_groups.push_back(ConnectionGroup(input_index, real_hidden_index, assign(key)));
//        if(_max_cppn_index_input_to_hidden < cppnIndex) _max_cppn_index_input_to_hidden = cppnIndex;
    }

    void connectHiddenToHidden(size_t hidden_index_1, size_t hidden_index_2){
        std::string key = getCppnOutputGroupKey(_hiddenToHiddenCppnIndex, hiddenLayer, hidden_index_1, hidden_index_2);
        size_t real_hidden_index_1 = hidden_index_1 + getNbInputGroups();
        size_t real_hidden_index_2 = hidden_index_2 + getNbInputGroups();
        _connection_groups.push_back(ConnectionGroup(real_hidden_index_1, real_hidden_index_2, assign(key)));
//        if(_max_cppn_index_hidden_to_hidden < cppnIndex) _max_cppn_index_hidden_to_hidden = cppnIndex;
    }

    void connectHiddenToOutput(size_t hidden_index, size_t output_index){
        std::string key = getCppnOutputGroupKey(_hiddenToOutputCppnIndex, outputLayer, hidden_index, output_index);
        size_t real_hidden_index = hidden_index + getNbInputGroups();
        size_t real_output_index = output_index + getNbInputGroups() + getNbHiddenGroups();
        _connection_groups.push_back(ConnectionGroup(real_hidden_index, real_output_index, assign(key)));
//        if(_max_cppn_index_hidden_to_output < cppnIndex) _max_cppn_index_hidden_to_output = cppnIndex;
    }

    void connectInputToHiddenFull(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Input to hidden: fully connected." << std::endl;
        for(size_t i=0; i<getNbInputGroups(); ++i){
            for(size_t j=0; j<getNbHiddenGroups(); ++j){
                connectInputToHidden(i,j);
            }
        }
    }

    void connectInputToHiddenOneToOne(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Input to hidden: one-to-one connected." << std::endl;
        dbg::assertion(DBG_ASSERTION(_inputToHiddenCppnIndex != combinatorialIndex));
        for(size_t i=0; i<getNbInputGroups(); ++i){
            connectInputToHidden(i,i);
        }
    }

    void connectHiddenToHiddenFull(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: fully connected." << std::endl;
        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            for(size_t j=0; j<getNbHiddenGroups(); ++j){
                connectHiddenToHidden(i,j);
            }
        }
    }

    void connectHiddenToHiddenWithin(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: within groups connected." << std::endl;
        dbg::assertion(DBG_ASSERTION(_hiddenToHiddenCppnIndex != combinatorialIndex));
        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            connectHiddenToHidden(i,i);
        }
    }

    void connectHiddenToOutputFull(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to output: fully connected." << std::endl;
        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            for(size_t j=0; j<getNbOutputGroups(); ++j){
                connectHiddenToOutput(i,j);
            }
        }
    }



    void setSeparateIndexPerLayer(bool flag){
        _separateOutputsPerLayer = flag;
    }

    void setSeparateIndexPerLayerNeurons(bool flag){
        _separateOutputsPerLayerNeurons = flag;
    }

    CppnIndex getHiddenCppnIndex(){
        return _hiddenCppnIndex;
    }

    void setHiddenCppnIndex(CppnIndex cppnIndex){
        _hiddenCppnIndex = cppnIndex;
    }

    CppnIndex getOutputCppnIndex(){
        return _outputCppnIndex;
    }

    void setOutputCppnIndex(CppnIndex cppnIndex){
        _outputCppnIndex = cppnIndex;
    }

    /**
     * Returns the general pattern for which CPPN output is responsible for which connection,
     * from the input layer to the hidden layer.
     *
     * @return An enumerator indicating the CPPN output pattern.
     */
    CppnIndex getInputToHiddenCppnIndex(){
        return _inputToHiddenCppnIndex;
    }

    /**
     * Sets the general pattern dictating which CPPN output is responsible for which connection,
     * from the input layer to the hidden layer.
     *
     * @param cppnIndex An enumerator indicating the CPPN output pattern.
     */
    void setInputToHiddenCppnIndex(CppnIndex cppnIndex){
        dbg::out(dbg::info, "neuron_groups") << "Input to hidden: " << getCppnOutputGroupPattern(cppnIndex) << std::endl;
        _inputToHiddenCppnIndex = cppnIndex;
    }

    /**
     * Returns the general pattern for which CPPN output is responsible for which connection,
     * from the hidden layer to the hidden layer.
     *
     * @return An enumerator indicating the CPPN output pattern.
     */
    CppnIndex getHiddenToHiddenCppnIndex(){
        return _hiddenToHiddenCppnIndex;
    }

    /**
     * Sets the general pattern dictating which CPPN output is responsible for which connection,
     * from the hidden layer to the hidden layer.
     *
     * @param cppnIndex An enumerator indicating the CPPN output pattern.
     */
    void setHiddenToHiddenCppnIndex(CppnIndex cppnIndex){
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: " << getCppnOutputGroupPattern(cppnIndex) << std::endl;
        _hiddenToHiddenCppnIndex = cppnIndex;
    }

    /**
     * Returns the general pattern for which CPPN output is responsible for which connection,
     * from the hidden layer to the output layer.
     *
     * @return An enumerator indicating the CPPN output pattern.
     */
    CppnIndex getHiddenToOutputCppnIndex(){
        return _hiddenToOutputCppnIndex;
    }

    /**
     * Sets the general pattern dictating which CPPN output is responsible for which connection,
     * from the hidden layer to the output layer.
     *
     * @param cppnIndex An enumerator indicating the CPPN output pattern.
     */
    void setHiddenToOutputCppnIndex(CppnIndex cppnIndex){
        dbg::out(dbg::info, "neuron_groups") << "Hidden to output: " << getCppnOutputGroupPattern(cppnIndex) << std::endl;
        _hiddenToOutputCppnIndex = cppnIndex;
    }

    static std::string getCppnOutputGroupPattern(CppnIndex cppnIndex){
        switch(cppnIndex){
        case staticIndex:
            return "static";
            break;
        case sourceIndex:
            return "source";
            break;
        case targetIndex:
            return "target";
            break;
        case combinatorialIndex:
            return "combinatorial";
            break;
        default:
            dbg::sentinel(DBG_HERE);
            return "";
        }
    }

    void build(){
        std::vector<NeuronGroup> allLayers;
        allLayers.insert(allLayers.end(), _inputLayer.begin(), _inputLayer.end());
        allLayers.insert(allLayers.end(), _hiddenLayer.begin(), _hiddenLayer.end());
        allLayers.insert(allLayers.end(), _outputLayer.begin(), _outputLayer.end());

        SFERES_INIT_NON_CONST_ARRAY_TEMPLATE(Params::ParamsHnn::multi_spatial, planes, allLayers.size());
        SFERES_INIT_NON_CONST_MATRIX_TEMPLATE(Params::ParamsHnn::multi_spatial, connected, allLayers.size(), allLayers.size());
        SFERES_INIT_NON_CONST_ARRAY_TEMPLATE(Params::ParamsHnn::multi_spatial, neuron_offsets, allLayers.size());
        SFERES_INIT_NON_CONST_ARRAY_TEMPLATE(Params::ParamsHnn::multi_spatial, connection_offsets, _connection_groups.size());

        //Initialize connected matrix with zeros
        for(size_t i=0; i<allLayers.size(); ++i){
            for(size_t j=0; j<allLayers.size(); ++j){
                Params::ParamsHnn::multi_spatial::connected_set(i, j, false);
            }
        }

        //Set planes
        for(size_t i=0; i<allLayers.size(); ++i){
            Params::ParamsHnn::multi_spatial::planes_set(i, allLayers[i].size());
            Params::ParamsHnn::multi_spatial::neuron_offsets_set(i, allLayers[i].offset());
        }

        for(size_t i=0; i<_connection_groups.size(); ++i){
//            size_t offset = base_offset + _connection_groups[i].getCppnConnectionOutputGroup() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
            Params::ParamsHnn::multi_spatial::connected_set(_connection_groups[i].source(), _connection_groups[i].target(), true);
            Params::ParamsHnn::multi_spatial::connection_offsets_set(i, _connection_groups[i].offset());
        }
#if defined(CTRNN_MULTIPLE_OUTPUTSETS)
        Params::ParamsHnn::dnn::nb_outputs = Params::ParamsHnn::cppn::nb_output_sets * phen::hnn::HnnCtrnnWinnerTakesAllConstants::nb_of_cppn_outputs;
#else
//        size_t nb_of_cppn_outputs_for_neurons = getNbCppnNeuronOutputGroups() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron;
//        size_t nb_of_cppn_outputs_for_connections = getNbCppnConnectionOutputGroups() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
        Params::ParamsHnn::dnn::nb_outputs = _nb_cppn_outputs;
#endif
    }

//    size_t getNbCppnConnectionOutputGroups(){
//        return _assigned_cppn_connection_output_groups.size();
//    }
//
//    void assignConnectionOutputGroupInputToHidden(size_t id){
//        size_t group_index = _cppn_connection_output_groups_input_to_hidden.size();
//        _cppn_connection_output_groups_input_to_hidden[id] = group_index;
//    }

    void print(){
        using namespace boost::spirit::karma;
        size_t offset;
        std::vector<NeuronGroup> allLayers;
        allLayers.insert(allLayers.end(), _inputLayer.begin(), _inputLayer.end());
        allLayers.insert(allLayers.end(), _hiddenLayer.begin(), _hiddenLayer.end());
        allLayers.insert(allLayers.end(), _outputLayer.begin(), _outputLayer.end());

        std::map<size_t, std::string> offset_neuron_map;
        std::map<size_t, std::set<std::pair<size_t, size_t> > > offset_con_map;
        std::cout << "Neuron groups total: " << allLayers.size() << std::endl;

        std::cout << std::endl;
        std::cout << "Input groups: " << _inputLayer.size() << std::endl;
        for(size_t i=0; i<_inputLayer.size(); ++i){
            offset = _inputLayer[i].offset();
            std::cout << " - input group: " << i <<
            		" size: " << _inputLayer[i].size() << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Hidden groups: " << _hiddenLayer.size() << std::endl;
        for(size_t i=0; i<_hiddenLayer.size(); ++i){
            offset = _hiddenLayer[i].offset();
            std::cout << " - hidden group: " << i <<
            		" size: " << _hiddenLayer[i].size() <<
					" offset: " << _hiddenLayer[i].offset() << std::endl;
            offset_neuron_map[offset] = offset_neuron_map[offset] + " " + _hiddenLayer[i].name();
        }

        std::cout << std::endl;
        std::cout << "Output groups: " << _outputLayer.size() << std::endl;
        for(size_t i=0; i<_outputLayer.size(); ++i){
            offset = _outputLayer[i].offset();
            std::cout << " - output group: " << i <<
            		" size: " << _outputLayer[i].size() <<
					" offset: " << _outputLayer[i].offset() << std::endl;
            offset_neuron_map[offset] = offset_neuron_map[offset] + " " + _outputLayer[i].name();
        }

        dbg::out(dbg::info, "neuron_groups") << "Connection groups: " << _connection_groups.size() << std::endl;
        for(size_t i=0; i<_connection_groups.size(); ++i){
            offset = _connection_groups[i].offset();
            std::string source_str = allLayers[_connection_groups[i].source()].name();
            std::string target_str = allLayers[_connection_groups[i].target()].name();
            dbg::out(dbg::info, "neuron_groups") << " - connection group: " << i
                    << " source: " << source_str << "(" << _connection_groups[i].source() << ")"
                    << " target: " << target_str << "(" << _connection_groups[i].target() << ")"
                    << " offset: " << _connection_groups[i].offset() << std::endl;
            offset_neuron_map[offset] = offset_neuron_map[offset] +
            		" con_" + boost::lexical_cast<std::string>(i) +
					"_s_" + source_str + "_t_" + target_str;
            offset_con_map[offset];
            offset_con_map[offset].insert(std::pair<size_t, size_t>(_connection_groups[i].source(), _connection_groups[i].target()));
        }

        size_t cppn_outputs = _nb_cppn_outputs;
        std::cout << std::endl;
        std::cout << "CPPN outputs: " << cppn_outputs << std::endl;

        std::cout << std::endl;
        std::cout << "Hidden layer:" << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], _hiddenLayer[j].offset()) << " ";
        }
        std::cout << std::endl;

        std::cout << std::endl;
        std::cout << "Output layer:" << std::endl;
        for(size_t j=0; j<_outputLayer.size(); ++j){
            std::cout << _outputLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_outputLayer.size(); ++j){
            std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], _outputLayer[j].offset()) << " ";
        }
        std::cout << std::endl;

        //Print Input to Hidden
        std::cout << std::endl;
        std::cout << "Input to hidden:" << std::endl;
        std::cout << "      ";
        for(size_t j=0; j<_inputLayer.size(); ++j){
            std::cout << _inputLayer[j].name() << "  ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
            for(size_t k=0; k<_inputLayer.size(); ++k){
                bool flag = true;
                for(size_t i = 0; i<cppn_outputs; ++i){
                    if(offset_con_map[i].count(std::pair<size_t, size_t>(k, j + _inputLayer.size()))){
                        std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], i) << " ";
                        flag = false;
                    }
                }
                if(flag) {
                    std::cout << " .  ";
                }
            }
            std::cout << std::endl;
        }

        //Print Hidden to Hidden
        std::cout << std::endl;
        std::cout << "Hidden to hidden:" << std::endl;
        std::cout << "     ";
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
            for(size_t k=0; k<_hiddenLayer.size(); ++k){
                bool flag = true;
                for(size_t i = 0; i<cppn_outputs; ++i){
                    if(offset_con_map[i].count(std::pair<size_t, size_t>(k + _inputLayer.size(), j + _inputLayer.size()))){
                        std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], i) << " ";
                        flag = false;
                    }
                }
                if(flag) {
                    std::cout << "  .  ";
                }
            }
            std::cout << std::endl;
        }

        //Print Hidden to Output
        std::cout << std::endl;
        std::cout << "Hidden to output:" << std::endl;
        std::cout << "     ";
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_outputLayer.size(); ++j){
            std::cout << _outputLayer[j].name() << " ";
            for(size_t k=0; k<_hiddenLayer.size(); ++k){
                bool flag = true;
                for(size_t i = 0; i<cppn_outputs; ++i){
                    if(offset_con_map[i].count(std::pair<size_t, size_t>(k + _inputLayer.size(), j + _hiddenLayer.size() + _inputLayer.size()))){
                        std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], i) << " ";
                        flag = false;
                    }
                }
                if(flag) {
                    std::cout << "  .  ";
                }
            }
            std::cout << std::endl;
        }
    }


private:
    std::vector<NeuronGroup> _inputLayer;
    std::vector<NeuronGroup> _hiddenLayer;
    std::vector<NeuronGroup> _outputLayer;
    std::vector<ConnectionGroup> _connection_groups;

//    size_t _max_cppn_index_hidden;
//    size_t _max_cppn_index_output;
//    size_t _max_cppn_index_input_to_hidden;
//    size_t _max_cppn_index_hidden_to_hidden;
//    size_t _max_cppn_index_hidden_to_output;
//
//    size_t _nb_cppn_neuron_groups_assigned_to_hidden_layer;
//    size_t _nb_cppn_neuron_groups_assigned_to_output_layer;


    bool _separateOutputsPerLayer;
    bool _separateOutputsPerLayerNeurons;

    CppnIndex _hiddenCppnIndex;
    CppnIndex _outputCppnIndex;
    CppnIndex _inputToHiddenCppnIndex;
    CppnIndex _hiddenToHiddenCppnIndex;
    CppnIndex _hiddenToOutputCppnIndex;

    std::map<std::string, size_t> _cppn_output_groups;
    size_t _nb_cppn_outputs;

    size_t assign(std::string key){
        if(_cppn_output_groups.count(key) == 0){
            _cppn_output_groups[key] = _nb_cppn_outputs;
            if(key.substr(0,3) == "con"){
                _nb_cppn_outputs += Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
            } else {
                _nb_cppn_outputs += Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron;
            }
        }
        return _cppn_output_groups[key];
    }

    std::string getCppnOutputGroupKey(CppnIndex cppnIndex, size_t layer, size_t source_index, size_t target_index){
        std::string key = "con_";

        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
       //greater than highest number used in the previous layer
        if(_separateOutputsPerLayer){
            key += boost::lexical_cast<std::string>(layer) + "_";
        }

        switch(cppnIndex){
        case staticIndex:
            key += "static";
            break;
        case sourceIndex:
            key += boost::lexical_cast<std::string>(source_index);
            break;
        case targetIndex:
            key += boost::lexical_cast<std::string>(target_index);
            break;
        case combinatorialIndex:
            key += boost::lexical_cast<std::string>(source_index) + "_" + boost::lexical_cast<std::string>(target_index);
            break;
        default:
            dbg::sentinel(DBG_HERE);
        }

        return key;
    }

    std::string getCppnOutputGroupKey(CppnIndex cppnIndex, size_t layer, size_t index){
        std::string key = "neu_";

        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
        //greater than highest number used in the previous layer
        if(_separateOutputsPerLayerNeurons){
            key += boost::lexical_cast<std::string>(layer) + "_";
        }

        switch(cppnIndex){
        case staticIndex:
            key += "static";
            break;
        case sourceIndex:
        case targetIndex:
        case combinatorialIndex:
            key += boost::lexical_cast<std::string>(index);
            break;
        default:
            dbg::sentinel(DBG_HERE);
        }

        return key;
    }
};



#endif /* EXP_MODULARITY_NEURON_GROUP_HPP_ */
