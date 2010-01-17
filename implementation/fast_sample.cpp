#include "fast_sample.h"

#include <sstream>

namespace Hex {

FastSample::FastSample(double *gammas) {
/* CAUTION!
 * FastSample assumes that FillArrays() and Rand::Init() have already been called
 */
	all_sum = 0.0;

	memset(_gammas, 0, kBoardSizeAligned * kBoardSizeAligned * sizeof(double));
	memset(_rows, 0, kBoardSizeAligned * sizeof(double));

	for (uint i = 0; i < kBoardSizeAligned * kBoardSizeAligned; ++i)
		_used_fields[i] = 1;

	if (gammas)
		FillGammas(gammas);
}

void FastSample::FillGammas(double *gammas)
{
	double sum;

	for (uint i = 0; i < kBoardSize; ++i) {
		sum = 0.0;
		for (uint j = 0; j < kBoardSize; ++j)
			sum += _gammas[(i + 1) * kBoardSizeAligned + 1 + j] = gammas[i * kBoardSize + j];
		_rows[i + 1] = sum;
		all_sum += sum;
	}

	return;
}

std::string FastSample::ToAsciiArt() {
	std::stringstream result;

	result.precision(4);
	result.width(6);

	/*
	for (uint i = 0; i < kBoardSize; ++i) {
		for (uint j = 0; j < kBoardSize; ++j)
			result << _gammas[(i + 1) * kBoardSizeAligned + 1 + j] << " ";
		result << " sum = " << _rows[i + 1] << "\n";
	}
	*/

	for (uint i = 0; i < kBoardSizeAligned; ++i) {
		for (uint j = 0; j < kBoardSizeAligned; ++j)
			result << _gammas[i * kBoardSizeAligned + j] << " ";
		result << " sum = " << _rows[i] << "\n";
	}

	result << "all_sum = " << all_sum << "\n";

	return result.str();
}

inline void FastSample::Change(uint position, double *new_gammas, bool) {
// position is in 0..(kBoardSize ^ 2)
	return Change(transf[position], new_gammas);
}

inline void FastSample::Change(uint normal_position, double *new_gammas) {
// normal_position in 0..(kBoardSizeAligned ^ 2)
	uint row = normal_rows[normal_position];

	_used_fields[normal_position] = 0;

	all_sum -= _rows[row - 1] + _rows[row] + _rows[row + 1];

	/* Decreasing row sums */
	_rows[row - 1] -=
		_gammas[normal_position - kBoardSizeAligned + 1] +
		_gammas[normal_position - kBoardSizeAligned];
	_rows[row] -=
		_gammas[normal_position + 1] + _gammas[normal_position] +
		_gammas[normal_position - 1];
	_rows[row + 1] -=
		_gammas[normal_position + kBoardSizeAligned] +
		_gammas[normal_position + kBoardSizeAligned - 1];
	/* Decreasing row sums */

	/* Changing gammas */
	_gammas[normal_position - kBoardSizeAligned + 1] =
		new_gammas[0] * _used_fields[normal_position - kBoardSizeAligned + 1];
	_gammas[normal_position + 1] = new_gammas[1] * _used_fields[normal_position + 1];
	_gammas[normal_position + kBoardSizeAligned] =
		new_gammas[2] * _used_fields[normal_position + kBoardSizeAligned];
	_gammas[normal_position] = 0.0;
	_gammas[normal_position + kBoardSizeAligned - 1] =
		new_gammas[3] * _used_fields[normal_position + kBoardSizeAligned - 1];
	_gammas[normal_position - 1] = new_gammas[4] * _used_fields[normal_position - 1];
	_gammas[normal_position - kBoardSizeAligned] =
		new_gammas[5] * _used_fields[normal_position - kBoardSizeAligned];
	/* Changing gammas */

	/* Zeroing out-of-bounds gammas */
	_gammas[(row - 1) * kBoardSizeAligned] = _gammas[row * kBoardSizeAligned - 1] =
		_gammas[row * kBoardSizeAligned] = _gammas[(row + 1) * kBoardSizeAligned - 1] =
		_gammas[(row + 1) * kBoardSizeAligned] = _gammas[(row + 2) * kBoardSizeAligned - 1] = 0.0f;
	/* Zeroing out-of-bounds gammas */

	/* Increasing row sums */
	_rows[row - 1] +=
		_gammas[normal_position - kBoardSizeAligned + 1] +
		_gammas[normal_position - kBoardSizeAligned];

	_rows[row] +=
		_gammas[normal_position + 1] +
		_gammas[normal_position - 1];

	_rows[row + 1] +=
		_gammas[normal_position + kBoardSizeAligned ] +
		_gammas[normal_position + kBoardSizeAligned - 1];

	/* w koncowych ruchach dokladnosc spada i zaczynaja sie dziac niemile rzeczy */
	if (_rows[row - 1] < 0.0)
		_rows[row - 1] = 0.0;
	if (_rows[row] < 0.0)
		_rows[row] = 0.0;
	if (_rows[row + 1] < 0.0)
		_rows[row + 1] = 0.0;
	/* Increasing row sums */

	_rows[0] = _rows[kBoardSizeAligned - 1] = 0.0;

	all_sum += _rows[row - 1] + _rows[row] + _rows[row + 1];

	return;
}

inline void FastSample::Change(uint row, uint column, double *new_gammas) {
//row, column in 0..(kBoardSize -1)
	return Change(transf[row * kBoardSize + column], new_gammas);
}

inline void FastSample::Change(const std::string &position, double *new_gammas) {
	return Change(transf[ParsePosition(position)], new_gammas);
}

inline uint FastSample::Random() const {
	/* returns random field from set
	[kBoardSizeAligned + 1, kBoardSize * kBoardSizeAligned + kBoardSize] \
	\ {i : i | kBoardSizeAligned or (i + 1) | kBoardSizeAligned} */
	
	double r = ((double) Rand::next_rand() / ((uint(1) << 31) - 1 - 1)) * all_sum;
	uint i, j;

	std::cout << r << std::endl;

	for (i = 1; i <= kBoardSize; ++i)
		if (r < _rows[i])
			break;
		else
			r -= _rows[i];

	i *= kBoardSizeAligned;

	for (j = 1; j <= kBoardSize; ++j)
		if (r < _gammas[i + j])
			break;
		else
			r -= _gammas[i + j];

	return i + j;
}

inline double FastSample::get_all_sum() const {
	return all_sum;
}

} // namespace Hex
