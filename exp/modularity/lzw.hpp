/*---------------------------------------------------------------------
Copyright (C) 2008 DigiPen Institute of Technology.
File Name: LZW.hpp
Author: Josh Hoida
---------------------------------------------------------------------*/

#ifndef LZW_HPP
#define LZW_HPP

#include <vector>
#include <stdlib.h>

#define MAXDICTIONARYSIZE	65534	//Value of USHORT(-1) - 1: for max dictionary Size
#define USHORTSENTINAL		65535	//Value of USHORT(-1):

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

//Structure that represents a value in the dictionary.
struct LZWDictionaryElement
{
	unsigned char *Value;
	unsigned int size;
};

//Structure to do LZW Coding with:
class LZWEncoding
{
public:
	LZWEncoding();
	~LZWEncoding();

	unsigned char *Encode(uchar * vals, unsigned int &Length);	//Encode the values using LZW
	unsigned char *Decode(uchar * vals, unsigned int &Length);	//Decode the values using LZW

	size_t dictionairySize(){ return Dictionary.size();};
	long getTotalBits(){ return totalBits;};

private:
	void AddToDictionary(unsigned char *vals, int length);		//Add a value into the dictionary.
	void AddToDictionary(std::vector<unsigned char> &String);	//Add a value into the dictionary.
	unsigned short BestMatch(unsigned char *String, int &size);	//Find the best match in the dictionary, Size is used to return length of match.
	void ClearDictionary(void);			//Clean out the dictionary.
	void InitializeDictionary(void);	//Add the 0 to 255 entries into the dictionary.

	std::vector<LZWDictionaryElement> Dictionary;	//Vector containing the dictionary entries.

	uint neededBits;
	uint maxBitCapacity;
	long totalBits;
};


#endif
