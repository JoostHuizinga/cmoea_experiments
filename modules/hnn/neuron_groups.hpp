/*
 * neuron_groups.hpp
 *
 *  Created on: Sep 4, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_NEURON_GROUPS_HPP_
#define MODULES_MISC_NEURON_GROUPS_HPP_

//Boost includes
#include <boost/spirit/include/karma.hpp>

//Sferes includes
#include <sferes/dbg/dbg.hpp>

//Module includes
#include <modules/datatools/params.hpp>


class NeuronGroup{
public:
    NeuronGroup(size_t size, bool is_input = false, size_t offset = 0, std::string prefix = "", size_t local_index = 0, size_t layer = 0, size_t index_in_layer=0, std::string postfix = ""):
        _size(size),
        _offset(offset),
        _is_input(is_input),
        _local_index(local_index),
        _layer(layer),
        _index_in_layer(index_in_layer),
        _postfix(postfix)
    {
        _name = prefix + boost::lexical_cast<std::string>(local_index);
    }
    size_t size() const{return _size;}
    size_t offset() const{return _offset;}
    bool is_input() const{return _is_input;}
    std::string name() const{return _name;}
    size_t layer() const{return _layer;}
    size_t index_in_layer() const{return _index_in_layer;}
    size_t local_index() const{return _local_index;}
    std::string postfix() const{return _postfix;}
private:
    size_t _size;
    size_t _offset;
    bool _is_input;
    std::string _name;
    size_t _local_index;
    size_t _layer;
    size_t _index_in_layer;
    std::string _postfix;
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


class NeuronGroupIterator{
public:
    NeuronGroupIterator(const std::vector<NeuronGroup>& neuronGroups):
        _neuron_groups(neuronGroups),
        _current_neuron_in_group(0),
        _current_group(0),
        _current_neuron(0)
    {}

    const NeuronGroup& next(){
        while(_neuron_groups[_current_group].size() <= _current_neuron_in_group){
            _current_neuron_in_group = 0;
            ++_current_group;
            dbg::assertion(DBG_ASSERTION(_current_group < _neuron_groups.size()));
        }
        ++_current_neuron_in_group;
        ++_current_neuron;
        return _neuron_groups[_current_group];
    }

    bool hasNext(){
        while((_current_group < _neuron_groups.size()) &&
                (_neuron_groups[_current_group].size() <= _current_neuron_in_group)){
            _current_neuron_in_group = 0;
            ++_current_group;
        }
        return _current_group < _neuron_groups.size();
    }

    size_t currentNeuron(){
        return _current_neuron;
    }

    size_t currentGroup(){
        return _current_group;
    }

private:
    const std::vector<NeuronGroup>& _neuron_groups;
    size_t _current_neuron_in_group;
    size_t _current_group;
    size_t _current_neuron;
};

class NeuronGroupsEnum{
public:
    enum CppnIndex{
        staticIndex,
        sourceIndex,
        targetIndex,
        combinatorialIndex,
        layeredSourceIndex
    };

    enum PostfixFlag{
        source,
        target,
        none
    };

    enum Layers{
        inputLayer = 0,
        hiddenLayer,
        outputLayer
    };
};


class NeuronGroups: public NeuronGroupsEnum{
public:
    NeuronGroups():
        _separateOutputsPerLayer(true),
        _separateOutputsPerLayerNeurons(true),
        _hiddenCppnIndex(sourceIndex),
        _outputCppnIndex(sourceIndex),
        _inputToHiddenCppnIndex(sourceIndex),
        _hiddenToHiddenCppnIndex(staticIndex),
        _hiddenToOutputCppnIndex(targetIndex),
        _postfixFlag(source),
        _nb_cppn_outputs(0),
        _nb_outputs_con(0),
        _nb_outputs_nue(0)
    {}

    /**
     * Adds an input group of size size_of_neuron_group.
     *
     * Substrate input groups are not associated with a CPPN neuron output group,
     * because substrate inputs do not require bias or time-constant information.
     */
    void addInput(size_t size_of_neuron_group, size_t layer = 0, std::string postfix = ""){
        dbg::out(dbg::info, "neuron_groups") << "Input neuron group added of size: " << size_of_neuron_group << std::endl;
        _inputLayer.push_back(NeuronGroup(size_of_neuron_group, true, 0, "in", _inputLayer.size(), layer, _inputLayer.size(), postfix));
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
    void addHidden(size_t size_of_neuron_group, size_t layer = 0, std::string postfix = ""){

        size_t index_in_layer = 0;
        if(_hiddenLayer.size() > 0 && _hiddenLayer[_hiddenLayer.size()-1].layer() == layer){
            index_in_layer = _hiddenLayer[_hiddenLayer.size()-1].index_in_layer() + 1;
        }
        dbg::out(dbg::info, "neuron_groups") << "Hidden neuron group added of size: " << size_of_neuron_group
                << " layer: " << layer
                << " index in layer: " << index_in_layer << std::endl;
        std::string key = getCppnOutputGroupKey(_hiddenCppnIndex, hiddenLayer, _hiddenLayer.size(), index_in_layer) + postfix;
        _hiddenLayer.push_back(NeuronGroup(size_of_neuron_group, false, assign(key), "hid", _hiddenLayer.size(), layer, index_in_layer, postfix));
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
    void addOutput(size_t size_of_neuron_group, size_t layer = 0, std::string postfix = ""){
        dbg::out(dbg::info, "neuron_groups") << "Output neuron group added of size: " << size_of_neuron_group << std::endl;
        std::string key = getCppnOutputGroupKey(_outputCppnIndex, outputLayer, _outputLayer.size(), _outputLayer.size()) + postfix;
        _outputLayer.push_back(NeuronGroup(size_of_neuron_group, false, assign(key), "out", _outputLayer.size(), layer, _outputLayer.size(), postfix));
    }


    NeuronGroupIterator getGroupIterator() const{
        return NeuronGroupIterator(_allGroups);
    }

    const ConnectionGroup& getConnectionGroup (size_t index) const{
        return _connection_groups[index];
    }

    size_t getNbInputGroups() const{ return _inputLayer.size();}
    size_t getNbHiddenGroups() const{ return _hiddenLayer.size();}
    size_t getNbOutputGroups() const{ return _outputLayer.size();}
    size_t getNbOfNeuronGroups() const{ return _allGroups.size();}
    size_t getNbOfConnectionGroups() const{ return _connection_groups.size();}

    size_t getNbOfInputs() const {
        size_t nbOfInputs = 0;
        for(size_t i=0; i<_inputLayer.size(); ++i){
            nbOfInputs += _inputLayer[i].size();
        }
        return nbOfInputs;
    }

    size_t getNbOfHidden() const {
        size_t nbOfInputs = 0;
        for(size_t i=0; i<_hiddenLayer.size(); ++i){
            nbOfInputs += _hiddenLayer[i].size();
        }
        return nbOfInputs;
    }

    size_t getNbOfOutputs() const {
        size_t nbOfInputs = 0;
        for(size_t i=0; i<_outputLayer.size(); ++i){
            nbOfInputs += _outputLayer[i].size();
        }
        return nbOfInputs;
    }

    void connectInputToHidden(size_t input_index, size_t hidden_index){
        std::string key = getCppnOutputGroupKey(_inputToHiddenCppnIndex, inputLayer, _inputLayer[input_index], _hiddenLayer[hidden_index]);
        size_t real_hidden_index = hidden_index + getNbInputGroups();
        _connection_groups.push_back(ConnectionGroup(input_index, real_hidden_index, assign(key)));
    }

    void connectHiddenToHidden(size_t hidden_index_1, size_t hidden_index_2){
        std::string key = getCppnOutputGroupKey(_hiddenToHiddenCppnIndex, hiddenLayer, _hiddenLayer[hidden_index_1], _hiddenLayer[hidden_index_2]);
        size_t real_hidden_index_1 = hidden_index_1 + getNbInputGroups();
        size_t real_hidden_index_2 = hidden_index_2 + getNbInputGroups();
        _connection_groups.push_back(ConnectionGroup(real_hidden_index_1, real_hidden_index_2, assign(key)));
    }

    void connectHiddenToOutput(size_t hidden_index, size_t output_index){
        std::string key = getCppnOutputGroupKey(_hiddenToOutputCppnIndex, outputLayer, _hiddenLayer[hidden_index], _outputLayer[output_index]);
        size_t real_hidden_index = hidden_index + getNbInputGroups();
        size_t real_output_index = output_index + getNbInputGroups() + getNbHiddenGroups();
        _connection_groups.push_back(ConnectionGroup(real_hidden_index, real_output_index, assign(key)));
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

    void connectLayeredFeedForwardFull(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: within groups connected." << std::endl;
        dbg::assertion(DBG_ASSERTION(_hiddenToHiddenCppnIndex != combinatorialIndex));
        for(size_t i=0; i<getNbInputGroups(); ++i){
            for(size_t j=0; j<getNbHiddenGroups(); ++j){
                if((_hiddenLayer[j].layer() - _inputLayer[i].layer()) == 1){
                    connectInputToHidden(i,j);
                }
            }
        }

        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            for(size_t j=0; j<getNbHiddenGroups(); ++j){
                if((_hiddenLayer[j].layer() - _hiddenLayer[i].layer()) == 1){
                    connectHiddenToHidden(i,j);
                }
            }
        }

        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            for(size_t j=0; j<getNbOutputGroups(); ++j){
                if((_outputLayer[j].layer() - _hiddenLayer[i].layer()) == 1){
                    connectHiddenToOutput(i,j);
                }
            }
        }
    }


    void connectLayeredFeedForwardOneToOne(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: within groups connected." << std::endl;
        dbg::assertion(DBG_ASSERTION(_hiddenToHiddenCppnIndex != combinatorialIndex));
        dbg::assertion(DBG_ASSERTION(getNbInputGroups() <= getNbHiddenGroups()));
        for(size_t i=0; i<getNbInputGroups(); ++i){
            if((_hiddenLayer[i].layer() - _inputLayer[i].layer()) == 1){
                connectInputToHidden(i,i);
            } else {
                std::cout << "WARNING: input group " << i << " has no corresponding group in hidden layer." << std::endl;
            }
        }

        typedef std::vector<size_t> layer_t;
        std::map<size_t, layer_t> layers;
        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            size_t current_layer = _hiddenLayer[i].layer();
            if(layers.count(current_layer) == 0){
                layers[current_layer] = std::vector<size_t>();
            }
            layers[current_layer].push_back(i);
        }

        std::vector<size_t> keys;
        for (std::map<size_t, layer_t>::iterator it=layers.begin(); it!=layers.end(); ++it){
            keys.push_back(it->first);
        }
        std::sort(keys.begin(), keys.end());

        for(size_t i=0; i<keys.size()-1; ++i){
            size_t key = keys[i];
            dbg::assertion(DBG_ASSERTION(i+1 < keys.size()));
            dbg::assertion(DBG_ASSERTION(key+1 == keys[i+1]));
            dbg::assertion(DBG_ASSERTION(layers.count(key) == 1));
            dbg::assertion(DBG_ASSERTION(layers.count(key+1) == 1));
            dbg::assertion(DBG_ASSERTION(layers[key].size() == layers[key+1].size()));
            for(size_t j=0; j<layers[key].size();++j){
                connectHiddenToHidden(layers[key][j],layers[key+1][j]);
            }
        }

        for(size_t i=0; i<getNbOutputGroups(); ++i){
            size_t hidden_index = _hiddenLayer.size()-getNbOutputGroups()+i;
            if((_outputLayer[i].layer() - _hiddenLayer[hidden_index].layer()) == 1){
                connectHiddenToOutput(hidden_index,i);
            } else {
                std::cout << "WARNING: output group " << i << " has no corresponding group in hidden layer." << std::endl;
            }
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

    void setNbOfCppnOutputsNeuron(size_t number_of_cppn_outputs){
        _nb_outputs_nue = number_of_cppn_outputs;
    }

    void setNbOfCppnOutputsCon(size_t number_of_cppn_outputs){
        _nb_outputs_con = number_of_cppn_outputs;
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

    void setPostfixFlag(PostfixFlag postfixFlag){
        _postfixFlag = postfixFlag;
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
        case layeredSourceIndex:
            return "layered source";
            break;
        default:
            dbg::sentinel(DBG_HERE);
            return "";
        }
    }

    void build(){
//        std::vector<NeuronGroup> allLayers;
        _allGroups.insert(_allGroups.end(), _inputLayer.begin(), _inputLayer.end());
        _allGroups.insert(_allGroups.end(), _hiddenLayer.begin(), _hiddenLayer.end());
        _allGroups.insert(_allGroups.end(), _outputLayer.begin(), _outputLayer.end());

//        SFERES_INIT_NON_CONST_ARRAY_TEMPLATE(Params::multi_spatial, planes, _allGroups.size());
//        SFERES_INIT_NON_CONST_MATRIX_TEMPLATE(Params::multi_spatial, connected, _allGroups.size(), _allGroups.size());
//        SFERES_INIT_NON_CONST_ARRAY_TEMPLATE(Params::multi_spatial, neuron_offsets, _allGroups.size());
//        SFERES_INIT_NON_CONST_ARRAY_TEMPLATE(Params::multi_spatial, connection_offsets, _connection_groups.size());
//
//        //Initialize connected matrix with zeros
//        for(size_t i=0; i<_allGroups.size(); ++i){
//            for(size_t j=0; j<_allGroups.size(); ++j){
//                Params::multi_spatial::connected_set(i, j, false);
//            }
//        }
//
//        //Set planes
//        for(size_t i=0; i<_allGroups.size(); ++i){
////            size_t offset = allLayers[i].getCppnNeuronOutputGroup() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron;
////            size_t offset = _cppn_output_groups[allLayers[i].getGroupKey()].offset();
//            Params::multi_spatial::planes_set(i, _allGroups[i].size());
//            Params::multi_spatial::neuron_offsets_set(i, _allGroups[i].offset());
//        }
//
//        for(size_t i=0; i<_connection_groups.size(); ++i){
////            size_t offset = base_offset + _connection_groups[i].getCppnConnectionOutputGroup() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
//            Params::multi_spatial::connected_set(_connection_groups[i].source(), _connection_groups[i].target(), true);
//            Params::multi_spatial::connection_offsets_set(i, _connection_groups[i].offset());
//        }
//#if defined(CTRNN_MULTIPLE_OUTPUTSETS)
//        Params::dnn::nb_outputs = Params::ParamsHnn::cppn::nb_output_sets * phen::hnn::HnnCtrnnWinnerTakesAllConstants::nb_of_cppn_outputs;
//#else
////        size_t nb_of_cppn_outputs_for_neurons = getNbCppnNeuronOutputGroups() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron;
////        size_t nb_of_cppn_outputs_for_connections = getNbCppnConnectionOutputGroups() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
////        Params::dnn::nb_outputs = _nb_cppn_outputs;
//#endif
    }

    size_t getNbCppnOutputs(){
        return _nb_cppn_outputs;
    }

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
            std::cout << " - input group: " << i << " size: " << _inputLayer[i].size() << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Hidden groups: " << _hiddenLayer.size() << std::endl;
        for(size_t i=0; i<_hiddenLayer.size(); ++i){
            offset = _hiddenLayer[i].offset();
            std::cout << " - hidden group: " << i << " size: " << _hiddenLayer[i].size() << " offset: " << _hiddenLayer[i].offset() << std::endl;
            offset_neuron_map[offset] = offset_neuron_map[offset] + " " + _hiddenLayer[i].name();
        }

        std::cout << std::endl;
        std::cout << "Output groups: " << _outputLayer.size() << std::endl;
        for(size_t i=0; i<_outputLayer.size(); ++i){
            offset = _outputLayer[i].offset();
            std::cout << " - output group: " << i << " size: " << _outputLayer[i].size() << " offset: " << _outputLayer[i].offset() << std::endl;
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
            offset_neuron_map[offset] = offset_neuron_map[offset] + " con_" + boost::lexical_cast<std::string>(i) + "_s_" + source_str + "_t_" + target_str;
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
    std::vector<NeuronGroup> _allGroups;
    std::vector<ConnectionGroup> _connection_groups;

    bool _separateOutputsPerLayer;
    bool _separateOutputsPerLayerNeurons;

    CppnIndex _hiddenCppnIndex;
    CppnIndex _outputCppnIndex;
    CppnIndex _inputToHiddenCppnIndex;
    CppnIndex _hiddenToHiddenCppnIndex;
    CppnIndex _hiddenToOutputCppnIndex;
    PostfixFlag _postfixFlag;

    std::map<std::string, size_t> _cppn_output_groups;
    size_t _nb_cppn_outputs;
    size_t _nb_outputs_con;
    size_t _nb_outputs_nue;

    size_t assign(std::string key){
        if(_cppn_output_groups.count(key) == 0){
            _cppn_output_groups[key] = _nb_cppn_outputs;
            if(key.substr(0,3) == "con"){
                _nb_cppn_outputs += _nb_outputs_con;
            } else {
                _nb_cppn_outputs += _nb_outputs_nue;
            }
        }
        return _cppn_output_groups[key];
    }

    std::string getCppnOutputGroupKey(CppnIndex cppnIndex, size_t layer, NeuronGroup& source_group, NeuronGroup& target_group){
        std::string key = "con_";

        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
       //greater than highest number used in the previous layer
        if(_separateOutputsPerLayer){
            key += boost::lexical_cast<std::string>(layer) + "_";
        }

        switch(cppnIndex){
        case staticIndex:
            dbg::out(dbg::info, "neuron_groups") << "static index"<< std::endl;
            key += "static";
            break;
        case sourceIndex:
            dbg::out(dbg::info, "neuron_groups") << "source index"<< std::endl;
            key += boost::lexical_cast<std::string>(source_group.local_index());
            break;
        case targetIndex:
            dbg::out(dbg::info, "neuron_groups") << "target index"<< std::endl;
            key += boost::lexical_cast<std::string>(target_group.local_index());
            break;
        case combinatorialIndex:
            dbg::out(dbg::info, "neuron_groups") << "source target index"<< std::endl;
            key += boost::lexical_cast<std::string>(source_group.local_index());
            key += "_" + boost::lexical_cast<std::string>(target_group.local_index());
            break;
        case layeredSourceIndex:
            dbg::out(dbg::info, "neuron_groups") << "layered source index"<< std::endl;
            key += boost::lexical_cast<std::string>(source_group.index_in_layer());
            break;
        default:
            dbg::sentinel(DBG_HERE);
        }

        switch(_postfixFlag){
        case source:
            key += source_group.postfix();
            break;
        case target:
            key += target_group.postfix();
            break;
        case none:
            //nop
            break;
        default:
            dbg::sentinel(DBG_HERE);
        }

        dbg::out(dbg::info, "neuron_groups") << "Key: " << key << std::endl;

        return key;
    }

    std::string getCppnOutputGroupKey(CppnIndex cppnIndex, size_t layer, size_t index, size_t index_in_layer){
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
        case layeredSourceIndex:
            key += boost::lexical_cast<std::string>(index_in_layer);
            break;
        default:
            dbg::sentinel(DBG_HERE);
        }

        return key;
    }
};




#endif /* MODULES_MISC_NEURON_GROUPS_HPP_ */
