#include "board.h"
#include "conditional_assert.h"
#include "positions.h"
#include <cstring>
#include <sstream>
#include <iostream>

#include "fast_sample.cpp"
#include "hash_board.cpp"
#include "pattern_data.cpp"


namespace Hex {

uint Rand::_seed;
PatternData pattern_data;

// -----------------------------------------------------------------------------

inline Player Player::First() { return Player(0); }

inline Player Player::Second() { return Player(1); }

inline Player Player::OfString (std::string player) {
	ASSERT(ValidPlayer(player));
	if (player == "black")
		return Player::First();
	else return Player::Second();
}

inline Player Player::Opponent() const {
	return Player(_val ^ 1);
}

inline bool Player::operator== (const Player& player) const {
	return player._val == _val;
}

inline bool Player::operator!= (const Player& player) {
	return player._val != _val;
}

inline Player::Player(uint val) : _val(val) {}

inline uint Player::GetVal() { return _val; }

inline bool Player::ValidPlayer(const std::string& player) {
	return player == "black" || player == "white";
}

// -----------------------------------------------------------------------------

inline Location Location::OfCoords (std::string coords) {
	ASSERT(ValidLocation(coords));
	uint x = coords[0] >= 'a' ? coords[0] - 'a' : coords[0] - 'A';
	uint y = coords[1] - '0';
	if (coords.size() > 2)
		y = y * 10 + coords[2] - '0';
	return Location(++x, y);
}

inline Location::Location(uint x, uint y) : _pos(ToTablePos(x, y)) { }
inline Location::Location(uint pos) : _pos(pos) {}

inline Location::Location() {}

inline uint Location::GetPos() const { return _pos; }

inline std::string Location::ToCoords() const {
	std::stringstream coords;
	coords << static_cast<char>(_pos % kBoardSizeAligned + 'a' - 1);
	coords << _pos / kBoardSizeAligned;
	return coords.str();
}

inline uint Location::ToTablePos(uint x, uint y) {
	ASSERT (ValidLocation(x, y));
	return y * (kBoardSizeAligned) + x;
}

inline bool Location::operator==(Location loc) const {
	return loc._pos == _pos;
}

inline bool Location::operator!=(Location loc) const {
	return loc._pos != _pos;
}

inline bool Location::ValidLocation(const std::string& location) {
	if (location.size() == 0 || location.size() > 3)
		return false;
	uint x = location[0] >= 'a' ? location[0] - 'a' : location[0] - 'A';
	uint y = location[1] - '0';
	if (location.size() > 2)
		y = y * 10 + location[2] - '0';
	return ValidLocation(++x, y);
}

inline bool Location::ValidPosition(uint pos) {
	uint x, y;
	ToCoords(pos, x, y);
	return ValidLocation(x, y);
}

inline bool Location::ValidLocation(uint x, uint y) {
	return x > 0 && y > 0 && x <= kBoardSize && y <= kBoardSize;
}

inline void Location::ToCoords(uint pos, uint& x, uint& y) {
	x = pos % kBoardSizeAligned;
	y = pos / kBoardSizeAligned;
}

// -----------------------------------------------------------------------------

inline Move::Move (const Player& player, const Location& location)
	: _player(player), _location(location) {}

inline Move::Move() :_player(Player::First()), _location(0) {}

inline Location Move::GetLocation() const { return _location; }

inline Player Move::GetPlayer() const { return _player; }

// -----------------------------------------------------------------------------

const uint Board::guarded_board_size = kBoardSize + 2;
const uint Board::table_size = kBoardSizeAligned * kBoardSizeAligned;

const Board Board::Empty() {

	Board board;

	FillArrays();

	uint counter = 0;
	for (uint i = 1; i <= kBoardSize; ++i) {
		for (uint j = 1; j <= kBoardSize; j++) {
			uint field = i * kBoardSizeAligned + j;
			board._fast_field_map[counter] = field;
			board._reverse_fast_field_map[field] = counter++;
		}
	}

	for (uint i = 0; i < table_size; i++)
		board._board[i] = 0;
	for (uint i = 1; i <= kBoardSize; ++i)
		board._board[i] = 1;
	for (uint i = (guarded_board_size - 1) * kBoardSizeAligned + 1;
			i < (guarded_board_size - 1) * (kBoardSizeAligned + 1); ++i) {
		board._board[i] = (guarded_board_size - 1) * kBoardSizeAligned + 1;
	}

	double red_gammas[kBoardSize * kBoardSize];
	double blue_gammas[kBoardSize * kBoardSize];
	for (uint i = 0; i < kBoardSize; ++i)
		for (uint j = 0; j < kBoardSize; ++j)
		{
			red_gammas[i * kBoardSize + j] = pattern_data.getRatio(board._hash_board.Hash(HBlack, i, j));
			blue_gammas[i * kBoardSize + j] = pattern_data.getRatio(board._hash_board.Hash(HWhite, i, j));
		}
	board._red_sample.FillGammas(red_gammas);
	board._blue_sample.FillGammas(blue_gammas);

	/*
	slowo komentarza:
	chodzi o to aby wypelnic poczatkowe wartosci gamma.
	_hash_board ma juz poczatkowe wartosci hashy dla pol (w wiekszosci sa to zera)
	metoda Hash dla pola i, j (wierwsz, kolumna) i gracza HBlack (czyli pierwszego)
	zwraca hash tego pola przy ruchu takiego gracza
	Pattern's_Gamma musi teraz zwocic gamme dla hasha
	dwa obiekty FastSample istnieja dlatego, ze dane pole ma inne prawdopodobienstwo
	zaleznie od gracza, ktory ma teraz zagrac
	*/

	return board;
}

inline Player Board::CurrentPlayer() const {
	return _current;
}

Move Board::RandomLegalMove (const Player& player) const {
	/* if you want to use values from patterns so badly that you don't care it doesn't work, use commented code below */
	/*
	if (player == Player::First())
		return Move(player,
				Location(_red_sample.Random()));
	else
		return Move(player,
				Location(_blue_sample.Random()));
	*/

	if (player == Player::First()){
		if (_red_sample.get_all_sum() > 1.0) 
			return Move(player, Location(_red_sample.Random()));
	}
	else{
		if (_blue_sample.get_all_sum() > 1.0) 
			return Move(player, Location(_blue_sample.Random()));
	}

	/* execution gets to this point only if gammas' sum is too small */
	return Move(player, Location(_fast_field_map[Rand::next_rand(_moves_left)]));
}

inline void Board::PlayLegal (const Move& move) {
/*
	std::cout << "Red Gammas:" << std::endl << _red_sample.ToAsciiArt() << std::endl;
	std::cout << "Blue Gammas:" << std::endl << _blue_sample.ToAsciiArt() << std::endl;
	std::cout << "Hash Board:" << std::endl << _hash_board.ToAsciiArt() << std::endl;
	std::cout << "Move:" << move.GetLocation().GetPos() << std::endl; 
*/
	ASSERT(IsValidMove(move));
	uint pos = move.GetLocation().GetPos();
	HashColor hc, next_hc;
	if (move.GetPlayer() == Player::First()) {
		_board[pos] = pos;
		MakeUnion(pos);
		hc = HBlack;
		next_hc = HWhite;
	} else {
		_board[pos] = -1;
		hc = HWhite;
		next_hc = HBlack;
	}
	uint fast_map_pos = _reverse_fast_field_map[pos];
	uint replace_pos = _fast_field_map[--_moves_left];
	_fast_field_map[fast_map_pos] = replace_pos;
	_reverse_fast_field_map[replace_pos] = fast_map_pos;
	_current = _current.Opponent();

	_hash_board.Change(hc, pos);
	uint *neighbours1 = _hash_board.NeighboursHash(next_hc, pos);
	uint *neighbours2 = _hash_board.NeighboursHash(hc, pos);
	/* w powyzszym bedzie musial takze pojawic sie pattern */

	/* zamiast 6 powinno pojawic sie cos w stylu pattern.size() */

	double *gammas1 = new double[6];
	double *gammas2 = new double[6];

	for (uint i = 0; i < 6; ++i)
	{
		gammas1[i] = pattern_data.getRatio(neighbours1[i]);
		gammas2[i] = pattern_data.getRatio(neighbours2[i]);
	}

	if (hc == HBlack)
	{
		_red_sample.Change(pos, gammas2);
		_blue_sample.Change(pos, gammas1);
	}
	else
	{
		_red_sample.Change(pos, gammas1);
		_blue_sample.Change(pos, gammas2);
	}

	delete [] gammas1;
	delete [] gammas2;
	delete [] neighbours1;
	delete [] neighbours2;

	/*
	slowo komentarza:
	caly proces losowego ruchu wyglada tak:
	0. inicjujemy _hash_board poczatkowymi hashami
		inicjujemy _red_sample i _blue_sample gammami odpowiadajacymi poczatkowym hashom
	1. losowanie ruchu: 
		jesli jest to gracz pierwszy losujemy z rozkladu czerwonych gamm, z niebieskich gamm jesli jest to gracz drugi
	2. zagranie ruchu:
		poza naturalnym odnowieniem stanu planszy, robimy jeszcze
		aktualizacje _hash_board, ktora polega na zmianie hashy wszystkich pol z patternu
		zbieramy wszystkie pola z patternu do tablicy neighbours (sasiadow umieszczamy w takiej kolejnosci w jakiej czytac bedzie FastSample)
		aktualizujemy stan _red_sample i _blue_sample
	*/

}

inline void Board::MakeUnion(uint pos) {
	uint rep = MakeUnion(pos, pos + 1);
	rep = MakeUnion(rep, pos - 1);
	rep = MakeUnion(rep, pos - kBoardSizeAligned);
	rep = MakeUnion(rep, pos - kBoardSizeAligned + 1);
	rep = MakeUnion(rep, pos + kBoardSizeAligned);
	MakeUnion(rep, pos + kBoardSizeAligned - 1);
}

uint Board::MakeUnion(uint pos1, uint pos2) {
	return _board[pos2] <= 0 ? pos1 : _board[pos1] = Find(pos2);
}

inline uint Board::Find(uint pos) {
	while (static_cast<uint>(_board[pos]) != pos)
		pos = _board[pos] = _board[_board[_board[pos]]];
	return pos;
}

inline uint Board::ConstFind(uint pos) const {
	while (static_cast<uint>(_board[pos]) != pos)
		pos = _board[pos];
	return pos;
}

inline bool Board::IsFull() const {
	return _moves_left == 0;
}

inline Player Board::Winner() const {
	if (ConstFind(1) ==
		ConstFind((guarded_board_size - 1) * kBoardSizeAligned + 1)) {
			return Player::First();
	}
	else return Player::Second();
}

inline void Board::Load (const Board& board) {
	memcpy(this, &board, sizeof(*this));
}

inline Board::Board() : _moves_left(kBoardSize * kBoardSize),
		_current(Player::First()) {
	Rand::init(time(NULL));
}

inline uint Board::MovesLeft() const {
	return _moves_left;
}

inline void Board::GetPossiblePositions(Board::ushort_ptr& locations) {
	locations = _fast_field_map;
}

std::string Board::ToAsciiArt(Location last_move) const {

	std::stringstream s;
	for (unsigned char x = 'a'; x < 'a' + kBoardSize; ++x)
		s << " " << x;
	s << std::endl;
	for (uint i = 1; i <= kBoardSize; ++i) {
		for (uint j = 1; j < (i < 10 ? i : i - 1); ++j)
			s << " ";
		s << i;
		if (i * kBoardSizeAligned + 1 == last_move.GetPos())
			s << "(";
		else s << " ";
		for (uint j = 1; j <= kBoardSize; ++j) {
			uint pos = i * kBoardSizeAligned + j;
			if (_board[pos] == 0)
				s << ".";
			else if (_board[pos] < 0)
				s << "#";
			else s << "O";
			if (pos == last_move.GetPos())
				s << ")";
			else if (pos + 1 == last_move.GetPos())
				s << "(";
			else s << " ";
		}
		s << i << std::endl;
	}
	for (uint j = 0; j <= kBoardSize; ++j)
		s << " ";
	for (unsigned char x = 'a'; x < 'a' + kBoardSize; ++x)
		s << " " << x;

	return s.str();
}

bool Board::IsValidMove(const Move& move) {
	if (!Location::ValidPosition(move.GetLocation().GetPos()))
		return false;
	return _board[move.GetLocation().GetPos()] == 0;
}

// -----------------------------------------------------------------------------

} // namespace Hex
