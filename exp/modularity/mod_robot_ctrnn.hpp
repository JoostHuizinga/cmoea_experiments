/*
 * mod_robot_ctrnn.h
 *
 *  Created on: Oct 16, 2013
 *      Author: joost
 */

#ifndef MOD_ROBOT_CTRNN_H_
#define MOD_ROBOT_CTRNN_H_

#include <vector>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>

namespace mod_robot{
class TanhNeuron{
	//The lamba used in the activation function
	SFERES_CONST float lambda = 5.0f;

public:
	TanhNeuron():bias(0),oldPotential(0),newPotential(0),output(0),updateFactor(1), _active(true){
		//nix
	}

	/**
	 * Returns the ouput of this neuron
	 */
	inline double getOutput() const{
		return output;
	}

	/**
	 * Sets the output of this neuron. Useful for inputs if you do not want to apply the activation function to it.
	 */
	inline void setOutput(const double& _output){
		output = _output;
	}

	/**
	 * Sets the bias of this neuron.
	 */
	inline void setBias(const double& _bias){
		bias = _bias;
	}

	/**
	 * Returns the bias of this neuron.
	 */
	inline double getBias() const{
		return bias;
	}

	/**
	 * Sets the time constant of this neuron.
	 */
	inline void setTimeConstant(const double& timeConstant){
#ifdef JHDEBUG
		if (timeConstant<1){
			std::cerr << "Warning: timeConstant is :" << timeConstant << " < 1" <<std::endl;
			print();
		}
#endif

		updateFactor = 1/timeConstant;
	}

	/**
	 * Returns the time constant of this neuron.
	 */
	inline double getTimeConstant() const{
		return 1/updateFactor;
	}

	/**
	 * Returns the potential of this neuron. The potential returned is the one associated with the current output.
	 */
	inline double getPotential() const{
		return oldPotential;
	}

	/**
	 * Updates the new potential based on the old potential and the incoming potential;
	 */
	inline void updateNewPotential(const double& incomingPotential){
		newPotential = oldPotential + updateFactor*(-oldPotential+incomingPotential);

#ifdef JHDEBUG
		if (std::isnan(newPotential)){
			std::cerr << "Warning: newPotential is nan" <<std::endl;
			std::cerr << "Incoming potential: " << incomingPotential <<std::endl;
			print();
		}
#endif
	}

	/**
	 * Updates the output based on the new potential and updates the oldPotential based on the new potential.
	 */
	inline void update(){
	    if(_active){
	        oldPotential = newPotential;
	        output = tanh((newPotential + bias) * lambda);
	    } else {
	        output = 0;
	    }

#ifdef JHDEBUG
		if (std::isnan(output)){
			std::cerr << "Warning: output is nan" <<std::endl;
			print();
		}
#endif
	}

	void print(){
		std::cerr << "bias: " << bias << std::endl;
		std::cerr << "oldPotential: " << oldPotential << std::endl;
		std::cerr << "newPotential: " << newPotential << std::endl;
		std::cerr << "output: " << output << std::endl;
		std::cerr << "updateFactor: " << updateFactor << std::endl;
	}

	inline void clear(){
		oldPotential = 0;
		newPotential = 0;
		output = 0;
	}

	inline void setActive(bool active=true){
	    _active=active;
	}

private:
	double bias;
	double oldPotential; //The potential, cell potential or voltage of the neuron at the previous timestep.
	double newPotential; //The potential, cell potential or voltage of the neuron at the new timestep.
	double output; //The actual output of the neuron, usually during the old timestep.
	double updateFactor;
	bool _active;
};


class SigmoidNeuron{
    //The lamba used in the activation function
    SFERES_CONST float lambda = 5.0f;

public:
    SigmoidNeuron():bias(0),oldPotential(0),newPotential(0),output(0),updateFactor(1), _active(true){
        //nix
    }

    /**
     * Returns the ouput of this neuron
     */
    inline double getOutput() const{
        return output;
    }

    /**
     * Sets the output of this neuron. Useful for inputs if you do not want to apply the activation function to it.
     */
    inline void setOutput(const double& _output){
        output = _output;
    }

    /**
     * Sets the bias of this neuron.
     */
    inline void setBias(const double& _bias){
        bias = _bias;
    }

    /**
     * Returns the bias of this neuron.
     */
    inline double getBias() const{
        return bias;
    }

    /**
     * Sets the time constant of this neuron.
     */
    inline void setTimeConstant(const double& timeConstant){
#ifdef JHDEBUG
        if (timeConstant<1){
            std::cerr << "Warning: timeConstant is :" << timeConstant << " < 1" <<std::endl;
            print();
        }
#endif

        updateFactor = 1/timeConstant;
    }

    /**
     * Returns the time constant of this neuron.
     */
    inline double getTimeConstant() const{
        return 1/updateFactor;
    }

    /**
     * Returns the potential of this neuron. The potential returned is the one associated with the current output.
     */
    inline double getPotential() const{
        return oldPotential;
    }

    /**
     * Updates the new potential based on the old potential and the incoming potential;
     */
    inline void updateNewPotential(const double& incomingPotential){
        newPotential = oldPotential + updateFactor*(-oldPotential+incomingPotential);

#ifdef JHDEBUG
        if (std::isnan(newPotential)){
            std::cerr << "Warning: newPotential is nan" <<std::endl;
            std::cerr << "Incoming potential: " << incomingPotential <<std::endl;
            print();
        }
#endif
    }

    /**
     * Updates the output based on the new potential and updates the oldPotential based on the new potential.
     */
    inline void update(){
        if(_active){
            oldPotential = newPotential;
            output = (tanh((newPotential + bias) * lambda) + 1) / 2.0;
        } else {
            output = 0;
        }

#ifdef JHDEBUG
        if (std::isnan(output)){
            std::cerr << "Warning: output is nan" <<std::endl;
            print();
        }
#endif
    }

    void print(){
        std::cerr << "bias: " << bias << std::endl;
        std::cerr << "oldPotential: " << oldPotential << std::endl;
        std::cerr << "newPotential: " << newPotential << std::endl;
        std::cerr << "output: " << output << std::endl;
        std::cerr << "updateFactor: " << updateFactor << std::endl;
    }

    inline void clear(){
        oldPotential = 0;
        newPotential = 0;
        output = 0;
    }

    inline void setActive(bool active=true){
        _active=active;
    }

private:
    double bias;
    double oldPotential; //The potential, cell potential or voltage of the neuron at the previous timestep.
    double newPotential; //The potential, cell potential or voltage of the neuron at the new timestep.
    double output; //The actual output of the neuron, usually during the old timestep.
    double updateFactor;
    bool _active;
};


template<typename Neuron>
class CTRNN{
public:
    typedef Neuron neuron_t;

	enum elementType{
		bias,
		potential,
		weight,
		ouput,
		timeConstant
	};

	CTRNN(const CTRNN& other){
		nrOfLayers = other.nrOfLayers;
		nrOfNeurons = other.nrOfNeurons;
		nrOfConnections = other.nrOfConnections;
		nrOfOuputs = other.nrOfOuputs;

		connections = new double[nrOfConnections];
		neurons = new neuron_t[nrOfNeurons];
		layersStart = new unsigned int[nrOfLayers+1];
		incommingConnectionsStart = new unsigned int[nrOfNeurons+1];

		for(unsigned int i=0; i<nrOfConnections; i++){
			connections[i]= other.connections[i];
		}

		for(unsigned int i=0; i<nrOfNeurons; i++){
			neurons[i]= other.neurons[i];
		}

		for(unsigned int i=0; i<nrOfLayers+1; i++){
			layersStart[i]= other.layersStart[i];
		}

		for(unsigned int i=0; i<nrOfNeurons+1; i++){
			incommingConnectionsStart[i]= other.incommingConnectionsStart[i];
		}
	}

	CTRNN(unsigned int inputs, unsigned int hidden, unsigned int ouputs){
		unsigned int layers[2];
		nrOfOuputs = ouputs;
		nrOfLayers = 2;
		layers[0]=inputs;
		layers[1]=hidden+ouputs;
		init(layers);
	}

	~CTRNN(){
		delete[] incommingConnectionsStart;
		delete[] connections;
		delete[] neurons;
		delete[] layersStart;
	}

	/**
	 * Set the connection between input neuron neuronIndexSource and hidden (or output) neuron neuronIndexTarget.
	 */
	void setConnectionInput(unsigned int neuronIndexSource, unsigned int neuronIndexTarget, double weight){
		setConnection(getConnectionIndexInput(neuronIndexSource, neuronIndexTarget), weight);
	}

	/**
	 * Set the connection between hidden (or output) neuron neuronIndexSource and hidden (or output) neuron neuronIndexTarget.
	 */
	void setConnectionHidden(unsigned int neuronIndexSource, unsigned int neuronIndexTarget, double weight){
		setConnection(getConnectionIndexHidden(neuronIndexSource, neuronIndexTarget), weight);
	}

	/**
	 * Sets the weight of the connection at index connectionIndex.
	 */
	void setConnection(unsigned int connectionIndex, double weight){
		dbg::out(dbg::info, "ctrnn") << "Connection set: " << connectionIndex << " weight: " << weight << std::endl;
		dbg::assertion(DBG_ASSERTION(connectionIndex < nrOfConnections));
		connections[connectionIndex] = weight;
	}

	/**
	 * Returns the weight of the indicated connection.
	 */
	inline const double& getConnection(const unsigned int& connectionIndex) const{
	    return connections[connectionIndex];
	}

	/**
	 * Returns the weight of the connection between the two indicated neurons.
	 */
    inline double getConnection(const unsigned int& neuronIndexSource, const unsigned int& neuronIndexTarget) const{
        if(neuronIndexTarget < getNumberOfInputs()) return 0.0;
        unsigned int connectionIndex = _getIncommingConnectionIndex(neuronIndexSource, neuronIndexTarget);
        dbg::out(dbg::info, "ctrnn") << "neuronIndexSource: " << neuronIndexSource <<
                " calculated index: " << getSourceIndex(connectionIndex) <<std::endl;
        dbg::out(dbg::info, "ctrnn") << "neuronIndexTarget: " << neuronIndexTarget <<
                " calculated index: " << getTargetIndex(connectionIndex) <<std::endl;
        dbg::assertion(DBG_ASSERTION(connectionIndex < getNumberOfConnections()));
        dbg::assertion(DBG_ASSERTION(neuronIndexSource == getSourceIndex(connectionIndex)));
        dbg::assertion(DBG_ASSERTION(neuronIndexTarget == getTargetIndex(connectionIndex)));
        return connections[connectionIndex];
    }

    /**
     * Returns the index of the source of the provided connection.
     */
    inline unsigned int getSourceIndex(const unsigned int& connectionIndex) const{
        dbg::assertion(DBG_ASSERTION(connectionIndex < getNumberOfConnections()));
        return connectionIndex % getNumberOfNeurons();
    }

    /**
     * Returns the index of the target of the provided connection.
     */
    inline unsigned int getTargetIndex(const unsigned int& connectionIndex) const{
        dbg::assertion(DBG_ASSERTION(connectionIndex < getNumberOfConnections()));
        return (connectionIndex / getNumberOfNeurons()) + getNumberOfInputs();
    }


	/**
	 * Set the value of the input at index neuronIndex
	 */
	inline void setInput(const unsigned int& neuronIndex,const double& value){
		dbg::out(dbg::info, "ctrnn") << "Input set: " << neuronIndex << " value: " << value << std::endl;
		dbg::assertion(DBG_ASSERTION(neuronIndex < nrOfNeurons));
		neurons[neuronIndex].setOutput(value);
	}


	/**
	 * Return a connection from the node at neuronIndexSource, to the node at neuronIndexTarget
	 */
	inline unsigned int getConnectionIndexHidden(const unsigned int& neuronIndexSource, const unsigned int& neuronIndexTarget) const{
		dbg::assertion(DBG_ASSERTION(neuronIndexSource < getNumberOfHiddenNeurons()));
		dbg::assertion(DBG_ASSERTION(neuronIndexTarget < getNumberOfHiddenNeurons()));
		return _getConnectionIndexWithinLayer(1, neuronIndexSource, neuronIndexTarget);
	}


	/**
	 * Return a connection from the node at neuronIndexSource in the input layer, to the node at neuronIndexTarget in the hidden layer
	 */
	inline unsigned int getConnectionIndexInput(const unsigned int& neuronIndexSource, const unsigned int& neuronIndexTarget) const{
		dbg::assertion(DBG_ASSERTION(neuronIndexSource < getNumberOfInputs()));
		dbg::assertion(DBG_ASSERTION(neuronIndexTarget < getNumberOfHiddenNeurons()));
		return _getIncommingConnectionIndex(1, neuronIndexSource, neuronIndexTarget);
	}


	/**
	 * Returns the overall index of the neuron at index neuronIndex in layer neuronLayer.
	 */
	inline unsigned int getNeuronIndex(const unsigned int& neuronLayer, const unsigned int& neuronOffset) const{
		dbg::out(dbg::info, "ctrnn") << "Neuron layer: " << neuronLayer
				<< " neuron offset: " << neuronOffset
				<< " layer start: " << layersStart[neuronLayer]
				<< " result: " << layersStart[neuronLayer] + neuronOffset << std::endl;
		dbg::assertion(DBG_ASSERTION(layersStart[neuronLayer] + neuronOffset < nrOfNeurons));
		return layersStart[neuronLayer] + neuronOffset;
	}


	/**
	 * Returns the neuron at index neuronIndex.
	 */
	inline neuron_t* getNeuron(const unsigned int& neuronIndex) const {
		dbg::assertion(DBG_ASSERTION(neuronIndex < nrOfNeurons));
		return &neurons[neuronIndex];
	}

	bool isInput(const unsigned int& neuronIndex) const {
	    return neuronIndex < getNumberOfInputs();
	}

	bool isOutput(const unsigned int& neuronIndex) const {
	    return neuronIndex >= getOutputsStart();
	}


	/**
	 * Returns a pointer to the hidden neuron at offset offset.
	 */
	inline neuron_t* getHiddenNeuron(const unsigned int& neuronOffset) const {
		return getNeuron(1, neuronOffset);
	}


    inline const neuron_t& getOutputNeuron(const unsigned int& neuronIndex) const{
        dbg::assertion(DBG_ASSERTION(getOutputsStart() + neuronIndex < nrOfNeurons));
        return neurons[getOutputsStart() + neuronIndex];
    }


	inline double getOutput(const unsigned int& neuronIndex) const{
		dbg::assertion(DBG_ASSERTION(getOutputsStart() + neuronIndex < nrOfNeurons));
		return neurons[getOutputsStart() + neuronIndex].getOutput();
	}


	inline unsigned int getNumberOfInputs() const{
		return layersStart[1];
	}


	inline unsigned int getNumberOfConnections() const{
		return nrOfConnections;
	}


	inline unsigned int getNumberOfNeurons() const{
		return nrOfNeurons;
	}


	inline unsigned int getNumberOfHiddenNeurons() const{
		return nrOfNeurons - getNumberOfInputs();
	}


	inline unsigned int getNumberOfLayers() const{
		return nrOfLayers;
	}


	inline unsigned int getNumberOfOutputs() const{
		return nrOfOuputs;
	}


	inline unsigned int getNumberOfIncommingConnections(const unsigned int& neuronIndex) const{
		dbg::assertion(DBG_ASSERTION(neuronIndex < nrOfNeurons));
		return (incommingConnectionsStart[neuronIndex+1]-incommingConnectionsStart[neuronIndex]);
	}


	inline void clear(){
		for(unsigned int i=0; i<nrOfNeurons; i++){
			neurons[i].clear();
		}
	}


	inline void reset(){
		clear();
        for(unsigned int i=0; i<nrOfNeurons; i++){
            neurons[i].setActive();
        }
	}


	/****************************************************
	 **************** Printing statements ***************
	 ****************************************************/

	void printWeights(){
		std::cout << "        :     0     1     2     3     4     5     6     7     8    10";
		printForEachNeuron(weight, getNumberOfInputs());
	}


	void printNeuronPotentials(){
		std::cout << "Printing potentials" <<std::endl;
		printPerLayer(potential);
	}


	void printNeuronOutputs(){
		std::cout << "Printing outputs" <<std::endl;
		printPerLayer(ouput);
	}


	void printNeuronBiases(){
		printForEachNeuron(bias);
	}


	void printIncommingConnectionStart(){
		for(unsigned int i=0; i<nrOfNeurons; i++){
			std::cout << "Neuron " << i << ": "<< incommingConnectionsStart[i] << std::endl;
		}
	}


	void printLayerStart(){
		for(unsigned int i=0; i<nrOfLayers+1; i++){
			std::cout << "Layer " << i << ": "<< layersStart[i] << std::endl;
		}
	}


	void activate(){
		unsigned int currentNeuron = 0;
		unsigned int inputNeuron = 0;

		unsigned int connection = 0;
		unsigned int previousLayerStart = 0;
		unsigned int currentLayerStart = 0;
		unsigned int currentLayerEnd = 0;

		//For each layer
		for(unsigned int currentLayer =1; currentLayer<nrOfLayers; currentLayer++){
			//Set layer starts
			previousLayerStart=layersStart[currentLayer-1];
			currentLayerStart=layersStart[currentLayer];
			currentLayerEnd=layersStart[currentLayer+1];

			//For the current layer
			currentNeuron = currentLayerStart;
			while(currentNeuron < currentLayerEnd){

				//Calculate the value of the current neuron
				inputNeuron = previousLayerStart;
//				neurons[currentNeuron].value = neurons[currentNeuron].bias;
//				std::cout << "Calculating neuron: " << currentNeuron << " start" << std::endl;

				//Using the CTRNN function described in:
				//Integrating Reactive, Sequential, and Learning Behavior Using Dynamical Neural Networks
				//Brian Yamauchi and Randall Beer (1994)
				//
				//DVi    1
				//--- = -- (-Vi + SUM(Wij*F(Vj)) + SUM(SijIj))
				// DT   Ti
				//
				//Notice that we do not handle inputs in a special way since our input neurons do not have any incoming connections.

				//SUM(Wij*F(Vj)) begin
				double incommingPotential = 0;
				while(inputNeuron < currentLayerEnd){
//					std::cout << "Input neuron: " << inputNeuron << " value: " << neurons[inputNeuron].value << " weight: " << connections[connection] <<  std::endl;
					dbg::assertion(DBG_ASSERTION(inputNeuron < nrOfNeurons));
					dbg::assertion(DBG_ASSERTION(connection < nrOfConnections));

					incommingPotential+=neurons[inputNeuron].getOutput()*connections[connection];

#ifdef JHDEBUG
                    if (std::isnan(incommingPotential)){
                        std::cerr << "Warning: potential is nan" <<std::endl;
                        std::cerr << "Potential: " << incommingPotential << std::endl;
                        std::cerr << "Neuron: " << inputNeuron << " output " << neurons[inputNeuron].getOutput() << std::endl;
                        std::cerr << "Connection: " << connection << " weight " << connections[connection] <<std::endl;
                    }
#endif

					connection++;
					inputNeuron++;
				}
				dbg::assertion(DBG_ASSERTION(currentNeuron < nrOfNeurons));
				neurons[currentNeuron].updateNewPotential(incommingPotential);

				dbg::out(dbg::info, "ctrnn") << "Updating layer: " << currentLayer << " neuron: " << currentNeuron << " value: " << neurons[currentNeuron].getOutput() << std::endl;

				//Goto the next neuron
				currentNeuron++;
			}
		}

		//For each neuron except the neurons in the first layer
		for(unsigned int currentNeuron = layersStart[1]; currentNeuron<nrOfNeurons; currentNeuron++){
			neurons[currentNeuron].update();
		}
	}

protected:
	//The arrays containing the weights, values and biases of the network
	double* connections;
	neuron_t* neurons;

	//Helper vectors to store the start and end positions of layers and outgoing connections
	unsigned int* layersStart;
	unsigned int* incommingConnectionsStart;

	//The number of layers, neurons and connections
	unsigned int nrOfLayers;
	unsigned int nrOfNeurons;
	unsigned int nrOfConnections;
	unsigned int nrOfOuputs;

	//The lamba used in the activation function
	SFERES_CONST float lambda = 5.0f;

private:
	/**
	 * Returns the neuron at index neuronIndex.
	 */
	inline neuron_t* getNeuron(const unsigned int& layer, const unsigned int& neuronOffset) const {
		return getNeuron(getNeuronIndex(layer, neuronOffset));
	}


	/**
	 * Returns the connection index from node source in the provided layer to node target in the provided layer.
	 */
	inline unsigned int _getConnectionIndexWithinLayer(const unsigned int& layer, const unsigned int& sourceOffset, const unsigned int& targetOffset ) const{
		return _getIncommingConnectionIndex(layer, sourceOffset + getLayerSize(layer-1), targetOffset);
	}


	/**
	 * Gets the connection index of the connection number connectionOffset from the neuron at layer layer with offset neuronOffset.
	 */
	inline unsigned int _getIncommingConnectionIndex(const unsigned int& layer, const unsigned int& sourceOffset, const unsigned int& targetOffset) const{
		dbg::assertion(DBG_ASSERTION(layer > 0));
		dbg::assertion(DBG_ASSERTION(layer < getNumberOfLayers()));
		dbg::assertion(DBG_ASSERTION(targetOffset < getNumberOfHiddenNeurons()));
		return _getIncommingConnectionIndex(sourceOffset, getNeuronIndex(layer, targetOffset));
	}


	/**
	 * Gets the connection index of 'sourceOffset' from the neuron at 'targetIndex'.
	 *
	 * This function is necessary because all connections are stored in a single array.
	 * This ctrnn is input based, meaning that neurons are associated with their incoming connections,
	 * rather than their outgoing connections.
	 * Let's assume that the target neuron has 10 incoming connections.
	 * If we want to select the 6th incoming connection to the target neuron,
	 * the 'sourceOffset' should be 6 and the 'targetIndex' should be the index of the target neurons.
	 * Now we find the were the connections to the target neuron start, and then we add 6 to get the 6th connection.
	 *
	 * Note that, in the default case, there will be 2 layers, one input layer and one 'main' layer.
	 * The 'main' layer contains both the hidden and the output neurons.
	 * Input neurons can only connect to neurons in the 'main' layer,
	 * but every neuron in the main layer can connect to every other neuron in the main layer.
	 * This means that every neuron in the 'main' layer has 'nr_of_inputs' + 'nr_of_main' number of incoming connections.
	 */
	inline unsigned int _getIncommingConnectionIndex(const unsigned int& sourceOffset, const unsigned int& targetIndex) const{
		dbg::out(dbg::info, "ctrnn") << "Neuron index: " << targetIndex << " offset " << sourceOffset << " connection start: " << incommingConnectionsStart[targetIndex] << std::endl;
		dbg::assertion(DBG_ASSERTION(targetIndex >= getNumberOfInputs())); //Input nodes have no incoming connections, so any index returned would be invalid.
		dbg::assertion(DBG_ASSERTION(targetIndex < getNumberOfNeurons()));
		dbg::assertion(DBG_ASSERTION(sourceOffset < getNumberOfIncommingConnections(targetIndex)));
		return incommingConnectionsStart[targetIndex] + sourceOffset;
	}


	/**
	 * Returns the number of neurons in the provided layer.
	 */
	inline unsigned int getLayerSize(const unsigned int& layer) const{
		return layersStart[layer+1] - layersStart[layer];
	}



	/**
	 * Return the index of the first output neuron.
	 */
	inline unsigned int getOutputsStart() const{
		return layersStart[nrOfLayers] - nrOfOuputs;
	}


	inline double getValue(unsigned int& neuronIndex) const{
		return neurons[neuronIndex].getOutput();
	}


	/**
	 * Initializes the network.
	 */
	void init(unsigned int* layers){
		nrOfNeurons=0;
		nrOfConnections=0;
		layersStart = new unsigned int[nrOfLayers+1];
		layersStart[0] = 0;

		//Set the layers of the network and calculate the total number of neurons.
		for(unsigned int i=0; i<nrOfLayers-1; i++){
			nrOfNeurons+=layers[i];
			//The number of layers is the number of incoming connection to this layer plus the number of connection within this layer.
			nrOfConnections+=layers[i]*layers[i+1] + layers[i+1]*layers[i+1];
			layersStart[i+1] = nrOfNeurons;
		}
		nrOfNeurons+=layers[nrOfLayers-1];
		layersStart[nrOfLayers] = nrOfNeurons;

		//Create the neurons and connections
		neurons = new neuron_t[nrOfNeurons];
		connections = new double[nrOfConnections];

		//Set the incoming connections array.
		incommingConnectionsStart = new unsigned int[nrOfNeurons+1];
		incommingConnectionsStart[0]=0;

		//The input nodes to not have any incoming connections
		for(unsigned int j=layersStart[0]; j<layersStart[1]+1; j++){
			incommingConnectionsStart[j]=0;
		}

		//For each layer
		for(unsigned int layer=0; layer<nrOfLayers-1; layer++){
			//From the start of that layer, to the end of that layer
			for(unsigned int j=layersStart[layer+1]; j<layersStart[layer+2]; j++){
				//The next connection starts one length of the previous layer + one length of the current layer
				incommingConnectionsStart[j+1]=incommingConnectionsStart[j]+layers[layer]+layers[layer+1];
			}
		}

		//Set all connections to zero
		for(unsigned int i = 0; i < nrOfConnections; i++){
			connections[i] = 0;
		}
	}


	/**
	 * Print something for each neuron.
	 * Options are:
	 * - bias: the bias of the neuron
	 * - value: the current value of the neuron
	 * - weight: the weights of all incoming connections
	 */
	void printPerLayer(enum elementType type, unsigned int currentNeuron = 0){
		std::cout << std::fixed << std::setprecision(2) << std::showpos;

		std::cout << "Inputs: ";
		for(unsigned int i=0; i<getNumberOfInputs(); i++){
			printForNeuron(i, type);
			std::cout << " ";
		}

		std::cout << std::endl << "Hidden: ";
		for(unsigned int i=getNumberOfInputs(); i<getOutputsStart(); i++){
			printForNeuron(i, type);
			std::cout << " ";
		}

		std::cout << std::endl << "Outputs: ";
		for(unsigned int i=getOutputsStart(); i<getNumberOfNeurons(); i++){
			printForNeuron(i, type);
			std::cout << " ";
		}
		std::cout << std::endl;
	}


	/**
	 * Print something for each neuron.
	 * Options are:
	 * - bias: the bias of the neuron
	 * - value: the current value of the neuron
	 * - weight: the weights of all incoming connections
	 */
	void printForEachNeuron(enum elementType type, unsigned int currentNeuron = 0){
		std::cout << std::fixed << std::setprecision(2) << std::showpos;
		for(unsigned int currentLayer = 0; currentLayer<nrOfLayers; currentLayer++){
			while(currentNeuron < layersStart[currentLayer+1]){
				std::cout << "Neuron " << currentNeuron << ": ";
				printForNeuron(currentNeuron, type);
				std::cout << std::endl;
				currentNeuron++;
			}
			std::cout <<  std::endl;
		}
	}


	/**
	 * Prints a specific neuron value.
	 * Options are:
	 * - bias: the bias of the neuron
	 * - value: the current value of the neuron
	 * - weight: the weights of all incoming connections
	 */
	void printForNeuron(unsigned int neuron, enum elementType type){
		switch(type){
			case bias:
				std::cout << neurons[neuron].getBias();
				break;
			case potential:
				std::cout << neurons[neuron].getPotential();
				break;
			case ouput:
				std::cout << neurons[neuron].getOutput();
				break;
			case weight:
				for(unsigned int i=incommingConnectionsStart[neuron]; i< incommingConnectionsStart[neuron+1]; i++){
					std::cout << connections[i] << " ";
				}
				break;
			default:
				std::cout << "WARNING: undefined type!" << std::endl;
		}
		std::cout << "";
	}
};
}



#endif /* MOD_ROBOT_CTRNN_H_ */
