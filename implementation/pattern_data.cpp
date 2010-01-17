/* for this class to work it is necessary that file pattern.data exist in the same directory */

#ifndef PATTERN_DATA_CPP_
#define PATTERN_DATA_CPP_

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "pattern_data.hpp"

using namespace std;

namespace Hex {

PatternData::PatternData() {
	fstream inputStream;
	int numberOfPatterns;
	int patternHash;
	int patternFound;
	int patternPicked;

	ratio = new double[MAX_HASH];
	found = new unsigned int[MAX_HASH];
	picked = new unsigned int[MAX_HASH];	
	
	/* array has to cleaned in case some pattern doesn't appear in the learning set and ratio array would contain some junk */
	
	for(int i=0; i<MAX_HASH; i++)
	{
		ratio[i] = 1.0;
		found[i] = 1;
		picked[i] = 1;	
	}
	
	inputStream.open("pattern.data");

	if (!inputStream) {
    		cerr << "Unable to open file with pattern data";
    		exit(1);
	}

	inputStream >> numberOfPatterns;

	for(int i=0; i<numberOfPatterns; i++){
		inputStream >> patternHash;
		inputStream >> patternPicked;
		inputStream >> patternFound;

		ratio[patternHash] = ((double)patternPicked)/((double)patternFound);
		found[patternHash] = patternFound;
		picked[patternHash] = patternPicked;
	}

	inputStream.close();
}

PatternData::~PatternData(){
	delete[] ratio;
	delete[] found;
	delete[] picked;
}

double PatternData::getRatio(int hash) {
	return ratio[hash];
}

unsigned int PatternData::getFound(int hash) {
	return found[hash];
}

unsigned int PatternData::getPicked(int hash) {
	return picked[hash];
}

} // namespace Hex

#endif
