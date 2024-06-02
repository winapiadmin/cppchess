#include "chess.h"

inline int square_file(int square) {
	/**
	 * Gets the file index of the square where 0 is the a-file.
	 */
	return square & 7;
}

inline int square_rank(int square) {
	/**
	 * Gets the rank index of the square where 0 is the first rank.
	 */

	return square >> 3;
}
inline int square_distance(int a, int b) {
	/**
	Gets the Chebyshev distance (i.e., the number of king steps) from square *a* to *b*.
	*/
	return std::max(abs(square_file(a) - square_file(b)), abs(square_rank(a) - square_rank(b)));
}
uint64_t _sliding_attacks(int square, uint64_t occupied, const std::vector<int>& deltas);
uint64_t _step_attacks(int square, const std::vector<int>& deltas);

std::vector<uint64_t> BB_KNIGHT_ATTACKS(64);
std::vector<uint64_t> BB_KING_ATTACKS(64);
std::vector<std::vector<uint64_t>> BB_PAWN_ATTACKS(2, std::vector<uint64_t>(64)); // color - COLORS
int _init_attacks() {
	for (int sq = 0; sq < 64; sq++) {
		BB_KNIGHT_ATTACKS[sq] = _step_attacks(sq, { 17, 15, 10, 6, -17, -15, -10, -6 });
		BB_KING_ATTACKS[sq] = _step_attacks(sq, { 9, 8, 7, 1, -9, -8, -7, -1 });
		BB_PAWN_ATTACKS[0][sq] = _step_attacks(sq, { -7, -9 });
		BB_PAWN_ATTACKS[1][sq] = _step_attacks(sq, { 7, 9 });
	}
	return 1;
}

int DUMMY = _init_attacks();

std::vector<uint64_t> BB_SQUARES = {
	1ULL << 0, 1ULL << 1, 1ULL << 2, 1ULL << 3, 1ULL << 4, 1ULL << 5, 1ULL << 6, 1ULL << 7,
	1ULL << 8, 1ULL << 9, 1ULL << 10, 1ULL << 11, 1ULL << 12, 1ULL << 13, 1ULL << 14, 1ULL << 15,
	1ULL << 16, 1ULL << 17, 1ULL << 18, 1ULL << 19, 1ULL << 20, 1ULL << 21, 1ULL << 22, 1ULL << 23,
	1ULL << 24, 1ULL << 25, 1ULL << 26, 1ULL << 27, 1ULL << 28, 1ULL << 29, 1ULL << 30, 1ULL << 31,
	1ULL << 32, 1ULL << 33, 1ULL << 34, 1ULL << 35, 1ULL << 36, 1ULL << 37, 1ULL << 38, 1ULL << 39,
	1ULL << 40, 1ULL << 41, 1ULL << 42, 1ULL << 43, 1ULL << 44, 1ULL << 45, 1ULL << 46, 1ULL << 47,
	1ULL << 48, 1ULL << 49, 1ULL << 50, 1ULL << 51, 1ULL << 52, 1ULL << 53, 1ULL << 54, 1ULL << 55,
	1ULL << 56, 1ULL << 57, 1ULL << 58, 1ULL << 59, 1ULL << 60, 1ULL << 61, 1ULL << 62, 1ULL << 63
};
constexpr uint64_t BB_EMPTY = 0;
constexpr uint64_t BB_ALL = 0xFFFFFFFFFFFFFFFF;

uint64_t BB_CORNERS = BB_SQUARES[0] | BB_SQUARES[7] | BB_SQUARES[56] | BB_SQUARES[63];
uint64_t BB_CENTER = BB_SQUARES[27] | BB_SQUARES[28] | BB_SQUARES[35] | BB_SQUARES[36];

uint64_t BB_LIGHT_SQUARES = 0x5555555555555555;
uint64_t BB_DARK_SQUARES = 0xAAAAAAAAAAAAAAAA;

std::vector<uint64_t> BB_FILES = {
	0x0101010101010101ULL << 0,
	0x0101010101010101ULL << 1,
	0x0101010101010101ULL << 2,
	0x0101010101010101ULL << 3,
	0x0101010101010101ULL << 4,
	0x0101010101010101ULL << 5,
	0x0101010101010101ULL << 6,
	0x0101010101010101ULL << 7
};

std::vector<uint64_t> BB_RANKS = {
	(uint64_t)0xFF << (8 * 0),
	(uint64_t)0xFF << (8 * 1),
	(uint64_t)0xFF << (8 * 2),
	(uint64_t)0xFF << (8 * 3),
	(uint64_t)0xFF << (8 * 4),
	(uint64_t)0xFF << (8 * 5),
	(uint64_t)0xFF << (8 * 6),
	(uint64_t)0xFF << (8 * 7)
};

uint64_t BB_BACKRANKS = BB_RANKS[0] | BB_RANKS[7];

std::vector<int> SQUARES_180 = {
	56, 57, 58, 59, 60, 61, 62, 63,
	48, 49, 50, 51, 52, 53, 54, 55,
	40, 41, 42, 43, 44, 45, 46, 47,
	32, 33, 34, 35, 36, 37, 38, 39,
	24, 25, 26, 27, 28, 29, 30, 31,
	16, 17, 18, 19, 20, 21, 22, 23,
	8, 9, 10, 11, 12, 13, 14, 15,
	0, 1, 2, 3, 4, 5, 6, 7
};
uint64_t _sliding_attacks(int square, uint64_t occupied, const std::vector<int>& deltas) {
	uint64_t attacks = BB_EMPTY;

	for (int delta : deltas) {
		int sq = square;

		while (true) {
			sq += delta;
			if (sq < 0 || sq >= 64 || square_distance(sq, sq - delta) > 2) {
				break;
			}

			attacks |= 1ULL<<sq;

			if (occupied & 1ULL<<sq) {
				break;
			}
		}
	}

	return attacks;
}

uint64_t _step_attacks(int square, const std::vector<int>& deltas) {
	return _sliding_attacks(square, BB_ALL, deltas);
}

uint64_t _edges(int square) {
	return ((BB_RANKS[0] | BB_RANKS[7]) & ~BB_RANKS[square_rank(square)]) |
		((BB_FILES[0] | BB_FILES[7]) & ~BB_FILES[square_file(square)]);
}

std::vector<uint64_t> _carry_rippler(uint64_t mask) {
	// Carry-Rippler trick to iterate subsets of mask.
	std::vector<uint64_t> subsets;
	uint64_t subset = 0;
	while (true) {
		subsets.push_back(subset);
		subset = (subset - mask) & mask;
		if (!subset) {
			break;
		}
	}
	return subsets;
}

std::pair<std::vector<uint64_t>, std::vector<std::unordered_map<uint64_t, uint64_t>>> _attack_table(const std::vector<int>& deltas) {
	std::vector<uint64_t> mask_table;
	std::vector<std::unordered_map<uint64_t, uint64_t>> attack_table;

	for (int square = 0; square < 64; square++) {
		std::unordered_map<uint64_t, uint64_t> attacks;

		uint64_t mask = _sliding_attacks(square, 0, deltas) & ~_edges(square);
		for (uint64_t subset : _carry_rippler(mask)) {
			attacks[subset] = _sliding_attacks(square, subset, deltas);
		}

		attack_table.push_back(attacks);
		mask_table.push_back(mask);
	}

	return { mask_table, attack_table };
}

std::pair<std::vector<uint64_t>, std::vector<std::unordered_map<uint64_t, uint64_t>>> BB_DIAG_MASKS_AND_ATTACKS = _attack_table({ -9, -7, 7, 9 });
std::pair<std::vector<uint64_t>, std::vector<std::unordered_map<uint64_t, uint64_t>>> BB_FILE_MASKS_AND_ATTACKS = _attack_table({ -8, 8 });
std::pair<std::vector<uint64_t>, std::vector<std::unordered_map<uint64_t, uint64_t>>> BB_RANK_MASKS_AND_ATTACKS = _attack_table({ -1, 1 });

std::vector<std::vector<uint64_t>> _rays() {
	std::vector<std::vector<uint64_t>> rays(64, std::vector<uint64_t>(64, BB_EMPTY));
	for (int a = 0; a < 64; a++) {
		for (int b = 0; b < 64; b++) {
			if (BB_DIAG_MASKS_AND_ATTACKS.second[a].count(BB_DIAG_MASKS_AND_ATTACKS.first[b])) {
				rays[a][b] = (BB_DIAG_MASKS_AND_ATTACKS.second[a][BB_DIAG_MASKS_AND_ATTACKS.first[b]] & BB_DIAG_MASKS_AND_ATTACKS.second[b][BB_DIAG_MASKS_AND_ATTACKS.first[b]]) | BB_SQUARES[a] | BB_SQUARES[b];
			}
			else if (BB_RANK_MASKS_AND_ATTACKS.second[a].count(BB_RANK_MASKS_AND_ATTACKS.first[b])) {
				rays[a][b] = BB_RANK_MASKS_AND_ATTACKS.second[a][BB_RANK_MASKS_AND_ATTACKS.first[b]] | BB_SQUARES[a];
			}
			else if (BB_FILE_MASKS_AND_ATTACKS.second[a].count(BB_FILE_MASKS_AND_ATTACKS.first[b])) {
				rays[a][b] = BB_FILE_MASKS_AND_ATTACKS.second[a][BB_FILE_MASKS_AND_ATTACKS.first[b]] | BB_SQUARES[a];
			}
		}
	}
	return rays;
}
std::vector<std::vector<uint64_t>> BB_RAYS = _rays(); //QRB

__int8 square(int rank, int file) { return 8 * rank + file; }
std::vector<int> getSetBitsIndices(uint64_t bitboard) {
	std::vector<int> bitIndexes;

	for (int i = 0; i < 64; ++i) {
		if ((bitboard >> i) & 1) {
			bitIndexes.push_back(i);
		}
	}

	return bitIndexes;
}

//BB_KNIGHT_ATTACKS[sq] = _step_attacks(sq, { 17, 15, 10, 6, -17, -15, -10, -6 });
//BB_KING_ATTACKS[sq] = _step_attacks(sq, { 9, 8, 7, 1, -9, -8, -7, -1 });
//BB_PAWN_ATTACKS[0][sq] = _step_attacks(sq, { -7, -9 });
//BB_PAWN_ATTACKS[1][sq] = _step_attacks(sq, { 7, 9 });
char board(char pos[8][8], int x, int y) {
	if (x >= 0 && x <= 7 && y >= 0 && y <= 7) {
		return pos[x][y];
	}
	return 'x';
}

int pinned_direction(char pos[8][8], std::pair<int, int> square) {
	if (string("PNBRQK").find(board(pos, square.first, square.second)) == std::string::npos) {
		return 0;
	}
	int color = 1;
	if (string("PNBRQK").find(board(pos, square.first, square.second)) == std::string::npos) {
		color = -1;
	}
	for (int i = 0; i < 8; i++) {
		int ix = (i + (i > 3)) % 3 - 1;
		int iy = (((i + (i > 3)) / 3) << 0) - 1;
		bool king = false;
		for (int d = 1; d < 8; d++) {
			char b = board(pos, square.first + d * ix, square.second + d * iy);
			if (b == 'K') {
				king = true;
			}
			if (b != '-') {
				break;
			}
		}
		if (king) {
			for (int d = 1; d < 8; d++) {
				char b = board(pos, square.first - d * ix, square.second - d * iy);
				if (b == 'q' || (b == 'b' && ix * iy != 0) || (b == 'r' && ix * iy == 0)) {
					return abs(ix + iy * 3) * color;
				}
				if (b != '-') {
					break;
				}
			}
		}
	}
	return 0;
}
int pinned(char pos[8][8], std::pair<int, int> square) {
	if (string("PNBRQK").find(board(pos, square.first, square.second)) == std::string::npos) {
		return 0;
	}
	return pinned_direction(pos, square) > 0 ? 1 : 0;
}
Board::Board()
{
	this->set_piece_at(0, ROOK, WHITE);
	this->set_piece_at(1, KNIGHT, WHITE);
	this->set_piece_at(2, BISHOP, WHITE);
	this->set_piece_at(3, QUEEN, WHITE);
	this->set_piece_at(4, KING, WHITE);
	this->set_piece_at(5, BISHOP, WHITE);
	this->set_piece_at(6, KNIGHT, WHITE);
	this->set_piece_at(7, ROOK, WHITE);
	this->set_piece_at(8, PAWN, WHITE);
	this->set_piece_at(9, PAWN, WHITE);
	this->set_piece_at(10, PAWN, WHITE);
	this->set_piece_at(11, PAWN, WHITE);
	this->set_piece_at(12, PAWN, WHITE);
	this->set_piece_at(13, PAWN, WHITE);
	this->set_piece_at(14, PAWN, WHITE);
	this->set_piece_at(15, PAWN, WHITE);
	this->set_piece_at(63, ROOK, BLACK);
	this->set_piece_at(62, KNIGHT, BLACK);
	this->set_piece_at(61, BISHOP, BLACK);
	this->set_piece_at(60, QUEEN, BLACK);
	this->set_piece_at(59, KING, BLACK);
	this->set_piece_at(58, BISHOP, BLACK);
	this->set_piece_at(57, KNIGHT, BLACK);
	this->set_piece_at(56, ROOK, BLACK);
	this->set_piece_at(55, PAWN, BLACK);
	this->set_piece_at(54, PAWN, BLACK);
	this->set_piece_at(53, PAWN, BLACK);
	this->set_piece_at(52, PAWN, BLACK);
	this->set_piece_at(51, PAWN, BLACK);
	this->set_piece_at(50, PAWN, BLACK);
	this->set_piece_at(49, PAWN, BLACK);
	this->set_piece_at(48, PAWN, BLACK);
	this->turn = WHITE;
	this->fullmove_clock = 1;
	this->ep_square = -1;
	this->castling_rights = 1 << 0 | 1 << 7 | 1 << 56 | 1 << 63;
}
bool Board::is_pinned(__int8 piece)
{/*
	PIECES pinner;
	if (piece + 8 == king || piece - 8 == king) {
		int file = square_file(piece);
		int rank = square_rank(king);
		for (int sq = square(0, file); sq < square(7, file); sq += 8) {
			if (this->rooks & sq)	pinner = ROOK;
			else if (this->queens & sq)  pinner = QUEEN;
			if (board[sq] != 'R' && board[sq] != 'Q') {
				pinner = NONE;
			}
		}
	}
	if (piece + 1 == king || piece - 1 == king) {
		int file = square_file(piece);
		int rank = square_rank(piece);
		for (int square = 8 * rank; square < 8 * (rank + 1); square++) {
			if (this->rooks & square)	pinner = ROOK;
			else if (this->queens & square)  pinner = QUEEN;
			if (board[square] != 'R' && board[square] != 'Q') {
				pinner = NONE;
			}
		}
	}
	return pinner;*/
	char board[64];
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			char piece_type = '.'; 
			if (auto piece_opt = piece_at(square(x, y)); piece_opt.has_value()) {
				piece_type = *piece_opt->symbol().c_str();
			}
			board[square(y, x)] = piece_type;
		}
	}
	// Create a new array for the rotated position
	char rotatedBoard[8][8];

	// Rotate the positions counterclockwise
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			rotatedBoard[j][7 - i] = board[square(i, j)] != '.' ? board[square(i, j)] : '-';
		}
	}
	return pinned(rotatedBoard, { square_file(piece), 7-square_rank(piece) });
}
/*string Board::str() {
	string board;
	for (int y = 0; y < 8; y++){
		for (int x = 0; x < 8; x++) {
			vector<char> PIECE_NAMES = { 0, 'P', 'N', 'B', 'R', 'Q', 'K' };
			char piece_type;
			if (occupied & 1 << square(x, y)){
				if (pawns & 1 << square(x, y)) piece_type = 'P';
				if (knights & 1 << square(x, y)) piece_type = 'N';
				if (bishops & 1 << square(x, y)) piece_type = 'B';
				if (rooks & 1 << square(x, y)) piece_type = 'R';
				if (queens & 1 << square(x, y)) piece_type = 'Q';
			}
			piece_type = (occupied_co[1] & 1 << square(x, y))?piece_type : tolower(piece_type);
			board += piece_type;
			board += " ";
		}
		board += "\n";
	}
	return board;
}*/
string Board::str() {

	std::string builder;

	for (int square : SQUARES_180) {
		optional<Piece> piece = piece_at(square);
		if (piece) {
			builder.push_back(*piece.value().symbol().c_str());
		}
		else {
			builder.push_back('.');
		}

		if (BB_SQUARES[square] & BB_FILES[7]) {
			if (square != 7) {
				builder.push_back('\n');
			}
		}
		else {
			builder.push_back(' ');
		}
	}

	return builder;
}
void Board::clear_board()
{
	occupied_co[0] = 0;
	occupied_co[1] = 0;
	occupied = 0;
	pawns = 0;
	knights = 0;
	queens = 0;
	bishops = 0;
	kings = 0;
	halfmove_clock = 0;
	fullmove_clock = 0;
	rooks = 0;
	turn = WHITE;
	castling_rights = 0;
}

void Board::set_piece_at(__int8 square, PieceType piece, Color color) {
	switch (piece) {
	case -1: break;
	case PAWN:
		pawns |= 1 << square;
		break;
	case KNIGHT:
		knights |= 1 << square;
		break;
	case BISHOP:
		bishops |= 1 << square;
		break;
	case ROOK:
		rooks |= 1 << square;
		break;
	case QUEEN:
		queens |= 1 << square;
		break;
	case KING:
		kings |= 1 << square;
		break;
	default:
		cerr << "Invalid piece";
		break;
	}
	occupied_co[!color] |= 1 << square;
	occupied |= 1 << square;
}
void Board::clean_castling_rights() {
	castling_rights = BB_EMPTY;
}
std::vector<string> Tokenize(const string str, const std::regex regex)
{

	std::vector<string> result;

	sregex_token_iterator it(str.begin(), str.end(), regex, -1);
	sregex_token_iterator reg_end;

	for (; it != reg_end; ++it) {
		if (!it->str().empty()) //token could be empty:check
			result.emplace_back(it->str());
	}

	return result;
}
std::vector<string> TokenizeDefault(const string str, string delim)
{

	regex re("[" + delim + "]+");

	return Tokenize(str, re);
}
string trim(string str) {
	const string whiteSpaces = " \t\n\r\f\v";
	// Remove leading whitespace
	size_t first_non_space = str.find_first_not_of(whiteSpaces);
	str.erase(0, first_non_space);
	// Remove trailing whitespace
	size_t last_non_space = str.find_last_not_of(whiteSpaces);
	str.erase(last_non_space + 1);
	return str;
}
void Board::set_board_fen(std::string fen) {
	const std::vector<std::string> PIECE_SYMBOLS = {
		"p", "n", "b", "r", "q", "k"
	};
	// Compatibility with set_fen().
	fen = trim(fen);
	if (fen.find(" ") != std::string::npos) {
		cerr << "expected position part of fen, got multiple parts: " << fen << endl;
	}

	// Ensure the FEN is valid.
	std::vector<std::string> rows = TokenizeDefault(fen, "/");
	if (rows.size() != 8) {
		cerr << "expected 8 rows in position part of fen (473): " << fen << endl;
	}

	// Validate each row.
	for (const auto& row : rows) {
		int field_sum = 0;
		bool previous_was_digit = false;
		bool previous_was_piece = false;

		for (char c : row) {
			auto v = std::find(PIECE_SYMBOLS.begin(), PIECE_SYMBOLS.end(), std::to_string(std::tolower(c)));
			if (std::isdigit(c)) {
				if (previous_was_digit) {
					cerr << "two subsequent digits in position part of fen: " << fen << endl;
				}
				field_sum += std::stoi(std::string(1, c));
				previous_was_digit = true;
				previous_was_piece = false;
			}
			else if (c == '~') {
				if (!previous_was_piece) {
					cerr << "'~' not after piece in position part of fen: " << fen << endl;
				}
				previous_was_digit = false;
				previous_was_piece = false;
			}
			else if (v <= PIECE_SYMBOLS.end()) {
				field_sum += 1;
				previous_was_digit = false;
				previous_was_piece = true;
			}
			else {
				cerr << "invalid character in position part of fen: " << fen << endl;
			}
		}

		if (field_sum != 8) {
			cerr << "expected 8 columns per row in position part of fen (510): " << fen << endl;
		}
	}

	// Clear the board.
	clear_board();

	// Put pieces on the board.
	int square_index = 0;
	for (char c : fen) {
		auto v = std::find(PIECE_SYMBOLS.begin(), PIECE_SYMBOLS.end(), std::to_string(std::tolower(c)));
		if (std::isdigit(c)) {
			square_index += std::stoi(std::string(1, c));
		}
		else if (v <= PIECE_SYMBOLS.end()) {
			if (c == '/') continue;
			Piece piece = Piece::from_symbol(std::string(1,c));
			set_piece_at(SQUARES_180[square_index], piece.piece_type, piece.color);
			square_index += 1;
		}
		else if (c == '~') {
			promoted |= BB_SQUARES[SQUARES_180[square_index - 1]];
		}
	}
}