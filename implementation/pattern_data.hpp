#ifndef PATTERN_DATA_HPP_
#define PATTERN_DATA_HPP_

#define MAX_HASH 100000

namespace Hex {

class PatternData {
public:
	PatternData();
	~PatternData();

	double getRatio(int);
	unsigned int getFound(int);
	unsigned int getPicked(int);

private:
	double* ratio;
	unsigned int* found;
	unsigned int* picked;
};

}
#endif
