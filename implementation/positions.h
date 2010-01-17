#ifndef POSITION_H_
#define POSITION_H_

typedef unsigned int uint;

namespace Hex {

const uint kBoardSize = 11;
const uint kBoardSizeAligned = 13;	// kBoardSize + 2

uint normal_rows[kBoardSizeAligned * kBoardSizeAligned];
uint transf[kBoardSize * kBoardSize];

void FillArrays() {
	for (uint i = 0; i < kBoardSize; ++i)
		for (uint j = 0; j < kBoardSize; ++j)
			transf[i * kBoardSize + j] = (i + 1) * kBoardSizeAligned + j + 1;

	for (uint i = 0; i < kBoardSizeAligned; ++i)
		for (uint j = 0; j < kBoardSizeAligned; ++j)
			normal_rows[i * kBoardSizeAligned + j] = i;
}

uint ParsePosition(const std::string &position) {
	uint row = position[0] - 'a';
	uint column = position.length() == 2 ? position[1] - '0' :
		(position[1] - '0') * 10 + position[2] - '0';

	return row * kBoardSize + column - 1;
}

}
#endif
