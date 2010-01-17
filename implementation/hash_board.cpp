#include "hash_board.h"

#include <sstream>
#include <iomanip>
#include <assert.h>

namespace Hex {

#define GUARD 3

HashBoard::HashBoard() {
	_total_size = kBoardSizeAligned * kBoardSizeAligned;
    memset(_played, 0, _total_size * sizeof(bool));

	uint _init_hash[_total_size];

	memset(_hash_board, 0,  _total_size * sizeof(uint));
	memset(_init_hash, 0, _total_size * sizeof(uint));

	for (uint i = 0; i < kBoardSizeAligned; ++i) {
		_hash_board[i] = _hash_board[_total_size - 1 - i] = 
			_init_hash[i] = _init_hash[_total_size - 1 - i] = GUARD;
        _played[i] = _played[_total_size - 1 - i] = true;
    }

	for (uint i = 1; i <= kBoardSize; ++i) {
		_hash_board[i * kBoardSizeAligned] =
			_hash_board[(i + 1) * kBoardSizeAligned - 1] = 
			_init_hash[i * kBoardSizeAligned] = 
			_init_hash[(i + 1) * kBoardSizeAligned - 1] = GUARD;
        _played[i * kBoardSizeAligned] = _played[(i+1) * kBoardSizeAligned - 1] = true;
    }

	for (uint i = 1; i <= kBoardSize; ++i) {
		InitHash(_init_hash, kBoardSizeAligned + i);
		InitHash(_init_hash, i * kBoardSizeAligned + 1);
		InitHash(_init_hash, (i + 1) * kBoardSizeAligned - 2);
		InitHash(_init_hash, _total_size - kBoardSizeAligned - 1 - i);
	}


	memcpy(_hash_board, _init_hash, _total_size * sizeof(uint));
}

HashColor HashBoard::ParseColor(const std::string &color) const
{
#ifdef NDEBUG
    if (color[0] == "w"){
        return HWhite;
    } else {
        return HBlack;
    }
#else
    if (color == "white"){
        return HWhite;
    } else if (color == "black"){
        return HBlack;
    } else {
        assert(false);
    }
#endif
}

void HashBoard::InitHash(uint *init, uint position) {
	init[position] =
		_hash_board[position - kBoardSizeAligned + 1] * 1 +
		_hash_board[position + 1] * 4 +
		_hash_board[position + kBoardSizeAligned] * 16 +
		_hash_board[position + kBoardSizeAligned - 1] * 64 +
		_hash_board[position - 1] * 256 +
		_hash_board[position - kBoardSizeAligned] * 1024;
    _played[position] = false;
}

std::string HashBoard::ToAsciiArt() {
	std::stringstream result;

	for (uint i = 0; i < kBoardSizeAligned; ++i) {
		for (uint j = 0; j < kBoardSizeAligned; ++j){
            int pos = i * kBoardSizeAligned + j;
			result << (_played[pos] ? "+":"-") << std::setw(9) << _hash_board[pos] << " ";
        }
		result << "\n";
	}

	result <<std::endl;

	return result.str();
}

inline void HashBoard::Change(HashColor color, uint position, bool) {
// position is in 0..(kBoardSize ^ 2)
	return Change(color, transf[position]);
}

inline void HashBoard::Change(HashColor color, uint normal_position) {
// normal_position in 0..(kBoardSizeAligned ^ 2)

	_hash_board[normal_position - kBoardSizeAligned + 1] += color * 64;
	_hash_board[normal_position + 1] += color * 256;
	_hash_board[normal_position + kBoardSizeAligned] += color * 1024;
	_hash_board[normal_position + kBoardSizeAligned - 1] += color * 1;
	_hash_board[normal_position - 1] += color * 4;
	_hash_board[normal_position - kBoardSizeAligned] += color * 16;
	_played[normal_position] = true;

	assert(normal_position + kBoardSizeAligned < kBoardSizeAligned * kBoardSizeAligned);

	return;
}

inline void HashBoard::Change(HashColor color, uint row, uint column) {
//row, column in 0..(kBoardSize -1)
	return Change(color, transf[row * kBoardSizeAligned + column]);
}

inline void HashBoard::Change(HashColor color, const std::string &position) {
	return Change(color, transf[ParsePosition(position)]);
}

inline void HashBoard::Change(const std::string &color, const std::string &position) {
	return Change(ParseColor(color), position);
}

inline uint HashBoard::Hash(HashColor color, uint position, bool) const {
// position is in 0..(kBoardSize ^ 2)
	return (_hash_board[transf[position]] << 2) | color;
}

inline uint HashBoard::Hash(HashColor color, uint normal_position) const {
// normal_position in 0..(kBoardSizeAligned ^ 2)
	return (_hash_board[normal_position] << 2) | color;
}

inline uint HashBoard::Hash(HashColor color, uint row, uint column) const {
//row, column in 0..(kBoardSize -1)
	return (_hash_board[transf[row * kBoardSize + column]] << 2) | color;
}

inline uint HashBoard::Hash(HashColor color, const std::string &position) const {
	return (_hash_board[transf[ParsePosition(position)]] << 2) | color;
}

inline uint HashBoard::Hash(const std::string &color, const std::string &position) const {
	return Hash(ParseColor(color), position);
}

inline uint * HashBoard::NeighboursHash(HashColor color, uint position, bool) const {
	return NeighboursHash(color, transf[position]);
}

inline uint * HashBoard::NeighboursHash(HashColor color, uint normal_position) const {
	uint * result = new uint[6];

	result[0] = (_hash_board[normal_position - kBoardSizeAligned + 1] << 2) | color;
	result[1] = (_hash_board[normal_position + 1] << 2) | color;
	result[2] = (_hash_board[normal_position + kBoardSizeAligned] << 2) | color;
	result[3] = (_hash_board[normal_position + kBoardSizeAligned - 1] << 2) | color;
	result[4] = (_hash_board[normal_position - 1] << 2) | color;
	result[5] = (_hash_board[normal_position - kBoardSizeAligned] << 2) | color;

	return result;
}

inline uint * HashBoard::NeighboursHash(HashColor color, uint row, uint column) const {
	return NeighboursHash(color, transf[row * kBoardSize + column]);
}

inline uint * HashBoard::NeighboursHash(HashColor color, const std::string &position) const {
	return NeighboursHash(color, transf[ParsePosition(position)]);
}

inline const uint* HashBoard::GetAllHashes() const {
    return _hash_board;
}

inline const bool* HashBoard::GetPositionsPlayed() const {
    return _played;
}

inline size_t HashBoard::GetBoardSize() const {
    return kBoardSizeAligned * kBoardSizeAligned;
}


} // namespace Hex
