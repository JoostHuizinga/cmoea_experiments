/*---------------------------------------------------------------------
Copyright (C) 2008 DigiPen Institute of Technology.
File Name: LZW.cpp
Author: Josh Hoida
---------------------------------------------------------------------*/

#include "lzw.hpp"
#include <string.h>

bool DictionaryMatch(LZWDictionaryElement * LS, unsigned char * RS, unsigned int MaxSize)
{
	//if the dictionary element is bigger than max size, then no.
	if(LS->size > MaxSize)
		return false;

	//otherwise iterate through the string.
	for(int i = 0; i < LS->size; i++)
	{
		if(LS->Value[i] != RS[i])	//If we find a mismatch
			return false;//BAIL!  it's not a match.
	}
	return true;//We found a match!
}

LZWEncoding::LZWEncoding()
{
	//Initialize the base dictionary.
	InitializeDictionary();
}

void LZWEncoding::InitializeDictionary(void)
{
	//Clear out the dictionary
	ClearDictionary();

	//initialize the dictionary to hold 0 - 255;
	unsigned char *Temp;
	Dictionary.resize(256);
	for(int i = 0; i < 256; i++)
	{
		//Create the character:
		Temp = new unsigned char;
		*Temp = uchar(i);
		//Set the array and size values.
		Dictionary[i].Value = Temp;
		Dictionary[i].size = 1;
	}

	neededBits = 9;
	maxBitCapacity = 512;
}

void LZWEncoding::ClearDictionary(void)
{
	//Clear out the
	for(unsigned int i = 0; i < Dictionary.size(); i++)
	{
		delete[] Dictionary[i].Value;
	}

	Dictionary.clear();
}
LZWEncoding::~LZWEncoding()
{
	//Clean out the dictionary.
	ClearDictionary();
}

void LZWEncoding::AddToDictionary(unsigned char *vals, int length)
{
	//if we're up to the max dictionary size, then stop adding new ones.
	if(Dictionary.size() == MAXDICTIONARYSIZE)
		return;

	LZWDictionaryElement NewElement;

	//set up the new dictionary element
	NewElement.size = length;			//Copy size
	NewElement.Value = new unsigned char [length];
	memcpy(NewElement.Value,vals,length);	//Create a copy of the string

	//Push it into the dictionary.
	Dictionary.push_back(NewElement);

	if(Dictionary.size() > maxBitCapacity){
		maxBitCapacity*=2;
		neededBits++;
	}
}

void LZWEncoding::AddToDictionary(std::vector<unsigned char> &String)
{
	//If we already have the max size, stop pushing in new values.
	if(Dictionary.size() == MAXDICTIONARYSIZE)
		return;

	//Create a new element:
	LZWDictionaryElement NewElement;

	NewElement.size = int(String.size());	//Copy the size
	NewElement.Value = new unsigned char [String.size()];	//Create a new string

	//Copy the values of the string
	for(unsigned int i = 0; i < String.size(); i++)
		NewElement.Value[i] = String[i];

	//Push it into the dictionary.
	Dictionary.push_back(NewElement);
}

unsigned short LZWEncoding::BestMatch(unsigned char *String, int &maxSize)
{
	unsigned short BestMatch = 0;				//Best Match's length
	unsigned short BestIndex = USHORTSENTINAL;	//Index of the best match

	//check for any 2nd level matches, since first level is guaranteed.
	for(unsigned int i = 256; i < Dictionary.size(); i++)
	{
		if(DictionaryMatch(&Dictionary[i], String, maxSize))
		{
			if(Dictionary[i].size > BestMatch)
			{
				BestMatch = Dictionary[i].size;
				BestIndex = i;
			}
		}
	}

	//If there was no match in the dictionary:
	if(BestIndex == USHORTSENTINAL)
	{
		maxSize = 1;
		return ushort(String[0]);	//Return the index of the start, IE first level match.

	}

	//Give back the index and length of the best match.
	maxSize = BestMatch;
	return BestIndex;

}

unsigned char *LZWEncoding::Encode(unsigned char * vals, unsigned int &Length)
{
	bool done = false;					//Are we there yet?!
	std::vector<unsigned short> Output;	//Encoded values for the stream.
	int DictionaryLength;				//Length of the dictionary element we just used.
	unsigned short DictionaryIndex;		//Index of the dictionary element we just used.

	totalBits = 0;

	//Go through all the values in input stream.
	for(unsigned int i = 0; i < Length;)
	{
		//Max length for the dictionary element is length left.
		DictionaryLength = Length - i;

		//get the best match so far.
		DictionaryIndex = BestMatch(vals + i, DictionaryLength);
		Output.push_back(DictionaryIndex);
		totalBits+=neededBits;

		//Are we done after this next match?
		if(Dictionary[DictionaryIndex].size <= Length - i)
		{
			AddToDictionary(vals + i, DictionaryLength+1);//NO?  Then add it to the dictionary.

		}
		//Move forward through the stream to the next spot.
		i+= DictionaryLength;
	}

	//We're done encoding, so let's copy them into the stream of shorts.
	unsigned short *UShortArray = new unsigned short [Output.size()];
	for(unsigned int i = 0; i < Output.size(); i++)
	{
		UShortArray[i] = Output[i];
	}

	Length = sizeof(short) * uint(Output.size());//Number of bytes in the stream, return as the resulting length
	return reinterpret_cast<unsigned char *>(UShortArray);	//return the array.
}

void DictionaryToVector(std::vector<unsigned char> &Vector, LZWDictionaryElement &D)
{
	//Push each of the dictionary values into the vector.
	for(int i = 0; i < D.size; i++)
	{
		Vector.push_back(D.Value[i]);
	}
}

void StringToOutput(std::vector<unsigned char> &Base,std::vector<unsigned char> &Addition)
{
	//Copy one vector into the other.
	for(unsigned int i = 0; i < Addition.size(); i++)
		Base.push_back(Addition[i]);
}

unsigned char *LZWEncoding::Decode(unsigned char * vals, unsigned int &Length)
{
	std::vector<unsigned char>Output;			//output to be copied for the stream later.
	unsigned short *Input = reinterpret_cast<unsigned short *>(vals);	//Change the unsigned char into unsigned shorts.
	unsigned int ShortLength = Length / 2;		//a short is 2 times larger than a char, so cut the length in half;
	std::vector<unsigned char> CurrentString,DictionaryEntry;
	unsigned short InputValue;

	//Initialize to the base dictionary [0 to 256]
	InitializeDictionary();

	//push back the dictionary value for this index (First char is always a level 1 match)
	Output.push_back(Dictionary[Input[0]].Value[0]);
	CurrentString.push_back(Output[0]);	//Current string to match.

	//Iterate through the input stream.
	for(unsigned int iter = 1; iter < ShortLength; iter++)
	{
		DictionaryEntry.clear();	//Clear out the current Dictionary Entry.

		//Get Current index.
		InputValue = Input[iter];

		//Is it already in the dictionary?
		if(InputValue < Dictionary.size())
		{
			//Copy the dictionary entry.
			DictionaryToVector(DictionaryEntry,Dictionary[InputValue]);
		}
		else if( InputValue == Dictionary.size())	//if I'm supposed to create a new dictionary element
		{											//With my currentString:
			DictionaryEntry = CurrentString;
			DictionaryEntry.push_back(CurrentString[0]);
		}

		StringToOutput(Output,DictionaryEntry);	//Copy the dictionary result into the output stream

		//Add the current string to the dictionary:
		CurrentString.push_back(DictionaryEntry[0]);
		AddToDictionary(CurrentString);

		//Put Copy the dictionary entry into the current string.
		CurrentString.clear();
		CurrentString = DictionaryEntry;
	}

	//Create and copy the output into a UCHAR array.
	unsigned char *FinalChar = new unsigned char [Output.size()];
	for(unsigned int iter = 0; iter < Output.size(); iter++)
		FinalChar[iter] = Output[iter];

	//Return length and the output.


	Length = uint(Output.size());
	return FinalChar;
}
