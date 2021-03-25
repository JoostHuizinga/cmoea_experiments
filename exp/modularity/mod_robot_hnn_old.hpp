/*
 * mod_robot_hnn.hpp
 *
 *  Created on: Sep 25, 2013
 *      Author: Joost Huizinga
 */

#ifndef DNN_H_
#define DNN_H_

#include <vector>
#include <iostream>
#include <iomanip>

namespace mod_robot{
struct Neuron{
	double bias;
	double value;
};

class DNN{


public:
	enum elementType{
		bias,
		value,
		weight
	};

	DNN(const DNN& other){
//		std::cout << "DNN Copying: " << other.connections << std::endl;
		nrOfLayers = other.nrOfLayers;
		nrOfNeurons = other.nrOfNeurons;
		nrOfConnections = other.nrOfConnections;

		connections = new double[nrOfConnections];
		neurons = new Neuron[nrOfNeurons];
		layersStart = new unsigned int[nrOfLayers+1];
		incommingConnectionsStart = new unsigned int[nrOfNeurons+1];

//		std::cout << "connections: " << nrOfConnections << " " <<  other.nrOfConnections << std::endl;
		for(unsigned int i=0; i<nrOfConnections; i++){
//			std::cout << connections << ": " << i << " " << other.connections << std::endl;
			connections[i]= other.connections[i];
		}

//		std::cout << "neurons" << std::endl;
		for(unsigned int i=0; i<nrOfNeurons; i++){
			neurons[i]= other.neurons[i];
		}

//		std::cout << "layers" << std::endl;
		for(unsigned int i=0; i<nrOfLayers+1; i++){
			layersStart[i]= other.layersStart[i];
		}

//		std::cout << "incomming connections" << std::endl;
		for(unsigned int i=0; i<nrOfNeurons+1; i++){
			incommingConnectionsStart[i]= other.incommingConnectionsStart[i];
		}

	}

	DNN(std::vector<unsigned int> layers){
		nrOfLayers = layers.size();
		init(layers.data());
	}

	DNN(unsigned int* layers, unsigned int nrOfLayers): nrOfLayers(nrOfLayers){
		init(layers);
	}

	~DNN(){
		delete[] incommingConnectionsStart;
		delete[] connections;
		delete[] neurons;
		delete[] layersStart;
	}

	void reset(){
		for(unsigned int i=0; i<nrOfConnections; i++){
//			std::cout << connections << ": " << i << " " << other.connections << std::endl;
			connections[i]= 0;
		}

//		std::cout << "neurons" << std::endl;
		for(unsigned int i=0; i<nrOfNeurons; i++){
			neurons[i].value = 0;
			neurons[i].bias = 0;
		}

	}

	void setBias(unsigned int neuronIndex, double bias){
		if(neuronIndex >= nrOfNeurons || neuronIndex < 0) std::cout << "Updating neuron: " << neuronIndex << " from: " << nrOfNeurons << std::endl;
		neurons[neuronIndex].bias = bias;
	}

	void setBias(const unsigned int& neuronLayer, const unsigned int& neuronIndex, const double& bias){
		setBias(layersStart[neuronLayer] + neuronIndex, bias);
	}

	void setConnection(unsigned int connectionIndex, double weight){
		if(connectionIndex >= nrOfConnections || connectionIndex < 0) std::cout << "Updating connection: " << connectionIndex << " from: " << nrOfConnections << std::endl;
		connections[connectionIndex] = weight;
	}

	void setConnection(unsigned int neuronIndexSource, unsigned int neuronIndexTarget, double weight){
		setConnection(incommingConnectionsStart[neuronIndexTarget] + neuronIndexSource, weight);
	}

	void setConnection(unsigned int sourceLayer, unsigned int neuronIndexSource, unsigned int neuronIndexTarget, double weight){
		setConnection(neuronIndexSource, layersStart[sourceLayer+1] + neuronIndexTarget, weight);
	}

	unsigned int getConnectionIndex(const unsigned int& neuronIndexSource, const unsigned int& neuronIndexTarget) const{
		std::cout << "getting connection index for source neuron: " << neuronIndexSource << " target neuron: " << neuronIndexTarget << " connection start: " << incommingConnectionsStart[neuronIndexTarget] << std::endl;
		return incommingConnectionsStart[neuronIndexTarget] + neuronIndexSource;
	}

	unsigned int getConnectionIndex(const unsigned int& sourceLayer, const unsigned int& neuronIndexSource, const unsigned int& neuronIndexTarget) const{
		std::cout << "getting connection index for layer: " << sourceLayer << " source neuron: " << neuronIndexSource << " target neuron: " << neuronIndexTarget << std::endl;
		return getConnectionIndex(neuronIndexSource, layersStart[sourceLayer+1] + neuronIndexTarget);
	}

	unsigned int getNeuronIndex(const unsigned int& neuronLayer, const unsigned int& neuronIndex) const{
		return layersStart[neuronLayer] + neuronIndex;
	}

	void setInput(unsigned int neuronIndex, double value){
		if (neuronIndex >= nrOfNeurons) std::cout << "setting input of neuron: " << neuronIndex << " from: " << nrOfNeurons << std::endl;
		neurons[neuronIndex].value = value;
	}

	double getValue(unsigned int neuronIndex){
		return neurons[neuronIndex].value;
	}

	double getOutput(unsigned int neuronIndex){
		unsigned int neuronNr = layersStart[nrOfLayers-1] + neuronIndex;
		if(neuronNr > nrOfNeurons) std::cout << "Accessing neuron: " << neuronNr << std::endl;
		return neurons[neuronNr].value;
	}

	unsigned int getNumberOfInputs(){
		return layersStart[1];
	}

	unsigned int getNumberOfConnections(){
		return nrOfConnections;
	}

	unsigned int getNumberOfNeurons(){
		return nrOfNeurons;
	}

	unsigned int getNumberOfLayers(){
		return nrOfLayers;
	}

	unsigned int getNumberOfOutputs(){
		return layersStart[nrOfLayers] - layersStart[nrOfLayers-1];
	}

	void printWeights(){
		printForEachNeuron(weight);
	}

	void printNeuronValues(){
		printForEachNeuron(value);
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

		for(unsigned int currentLayer =1; currentLayer<nrOfLayers; currentLayer++){
			//Set layer starts
			previousLayerStart=layersStart[currentLayer-1];
			currentLayerStart=layersStart[currentLayer];
			currentLayerEnd=layersStart[currentLayer+1];

			currentNeuron = currentLayerStart;
			while(currentNeuron < currentLayerEnd){

				//Calculate the value of the current neuron
				inputNeuron = previousLayerStart;

				if (currentNeuron >= nrOfNeurons) std::cout << "Activating neuron: " << currentNeuron << " of: " << nrOfNeurons << std::endl;
				neurons[currentNeuron].value = neurons[currentNeuron].bias;
//				std::cout << "Calculating neuron: " << currentNeuron << " start" << std::endl;
				while(inputNeuron < currentLayerStart){
//					std::cout << "Input neuron: " << inputNeuron << " value: " << neurons[inputNeuron].value << " weight: " << connections[connection] <<  std::endl;
					neurons[currentNeuron].value+=neurons[inputNeuron].value*connections[connection];
					connection++;
					inputNeuron++;
				}
//				std::cout << "Calculating neuron: " << currentNeuron << " done"<< std::endl;
				neurons[currentNeuron].value = tanh(neurons[currentNeuron].value * lambda);

				//Goto the next neuron
				currentNeuron++;
			}
		}
	}

protected:
	//The arrays containing the weights, values and biases of the network
	double* connections;
	Neuron* neurons;

	//Helper vectors to store the start and end positions of layers and outgoing connections
	unsigned int* layersStart;
	unsigned int* incommingConnectionsStart;

	//The number of layers, neurons and connections
	unsigned int nrOfLayers;
	unsigned int nrOfNeurons;
	unsigned int nrOfConnections;

	//The lamba used in the activation function
	static const float lambda = 5.0f;

private:
	void init(unsigned int* layers){
		nrOfNeurons=0;
		nrOfConnections=0;


		layersStart= new unsigned int[nrOfLayers+1];
		layersStart[0]=0;


		for(unsigned int i=0; i<nrOfLayers-1; i++){
			nrOfNeurons+=layers[i];
			nrOfConnections+=layers[i]*layers[i+1];
			layersStart[i+1] = nrOfNeurons;
		}
		nrOfNeurons+=layers[nrOfLayers-1];
		layersStart[nrOfLayers] = nrOfNeurons;

		neurons = new Neuron[nrOfNeurons];
		connections = new double[nrOfConnections];
		incommingConnectionsStart = new unsigned int[nrOfNeurons+1];
		incommingConnectionsStart[0]=0;

		//The input nodes to not have any incoming connections
		for(unsigned int j=layersStart[0]; j<layersStart[1]+1; j++){
			incommingConnectionsStart[j]=0;
			std::cout << "Incomming connection: " << j << ": " << incommingConnectionsStart[j] << std::endl;
		}



		//For each layer
		for(unsigned int layer=0; layer<nrOfLayers-1; layer++){
			//From the start of that layer, to the end of that layer
			for(unsigned int j=layersStart[layer+1]; j<layersStart[layer+2]; j++){
				//The next connection starts one length of the next layer away from the current connection
				incommingConnectionsStart[j+1]=incommingConnectionsStart[j]+layers[layer];
				std::cout << "Incomming connection: " << j+1 << ": " << incommingConnectionsStart[j+1] << std::endl;
			}
		}

		//The output nodes do not have any outgoing connections
//		for(unsigned int j=layersStart[nrOfLayers-1]; j<layersStart[nrOfLayers]; j++){
//			incommingConnectionsStart[j]=nrOfConnections;
//		}
	}

	void printForEachNeuron(enum elementType type){
		std::cout << std::fixed << std::setprecision(2);
		unsigned int currentNeuron = 0;
		for(unsigned int currentLayer = 0; currentLayer<nrOfLayers; currentLayer++){
			while(currentNeuron < layersStart[currentLayer+1]){
				printForNeuron(currentNeuron, type);
				currentNeuron++;
			}
			std::cout <<  std::endl;
		}
	}

	void printForNeuron(unsigned int neuron, enum elementType type){
		switch(type){
			case bias:
				std::cout << neurons[neuron].bias;
				break;
			case value:
				std::cout << neurons[neuron].value;
				break;
			case weight:
				for(unsigned int i=incommingConnectionsStart[neuron]; i< incommingConnectionsStart[neuron+1]; i++){
					std::cout << connections[i] << " ";
				}
				break;
			default:
				std::cout << "WARNING: undefined type!" << std::endl;
		}
		std::cout << " | ";
	}
};
}

#endif /* DNN_H_ */
