#pragma once
#include <cstdint>
#include <map>
#include <tuple>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <iterator>
#include <regex>
#include <optional>
#include <variant>
using namespace std;
typedef __int64 Bitboard;

enum PieceType {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum Color {
    WHITE,
    BLACK
};
/*uint64_t sum(std::vector<uint64_t> a) {
	uint64_t t = 0;
	for (uint64_t x : a) {
		t += x;
	}
	return t;
}*/
/**/
class Piece {
public:
    Piece(PieceType piece_type, Color color)
        : piece_type(piece_type), color(color) {}

    std::string symbol() const {
        std::string symbol = piece_symbol(piece_type);
        return (color == Color::WHITE) ? std::string(1, std::toupper(symbol[0])) : symbol;
    }

    static Piece from_symbol(const std::string& symbol) {
        std::unordered_map<char, PieceType> PIECE_SYMBOLS = {
            {'p', PieceType::PAWN },
            {'n', PieceType::KNIGHT},
            {'b', PieceType::BISHOP },
            {'r', PieceType::ROOK},
            {'q', PieceType::QUEEN},
            {'k', PieceType::KING}
        };

        char lowercase_symbol = std::tolower(symbol[0]);
        if (PIECE_SYMBOLS.find(lowercase_symbol) != PIECE_SYMBOLS.end()) {
            PieceType piece_type = PIECE_SYMBOLS[lowercase_symbol];
            Color color = (symbol[0] == lowercase_symbol) ? Color::WHITE : Color::BLACK;
            return Piece(piece_type, color);
        }
        else{
            cerr << "Not found piece " << lowercase_symbol << " original: " << symbol << endl;
            return Piece(PAWN, WHITE);
        }
    }

    static std::string piece_symbol(PieceType piece_type) {
        switch (piece_type) {
        case PieceType::PAWN:   return "p";
        case PieceType::KNIGHT: return "n";
        case PieceType::BISHOP: return "b";
        case PieceType::ROOK:   return "r";
        case PieceType::QUEEN:  return "q";
        case PieceType::KING:   return "k";
        }
        return "";
    }
    PieceType piece_type;
    Color color;
};

class Board {
public:
	Board();
	Bitboard occupied;
	Bitboard occupied_co[2];
	Bitboard castling_rights;
	Bitboard pawns;
	Bitboard queens;
	Bitboard knights;
	Bitboard kings;
	Bitboard bishops;
	Bitboard rooks;
	Bitboard promoted;
	int halfmove_clock, fullmove_clock;
    Color turn;
	int ep_square;
	bool is_pinned(__int8 piece);
	//string str();
	void set_piece_at(__int8 square, PieceType piece, Color color);
	void clear_board();
	void clean_castling_rights();
	void set_board_fen(std::string fen);
    optional<Piece> piece_at(int square) {
        optional<PieceType> piece_type = piece_type_at(square);
        if (piece_type) {
            Color color = (Color)(occupied_co[0] & 1ULL<<square);
            return Piece(piece_type.value(), color);
        }
        else {
            return nullopt;
        }
    }

    optional<PieceType> piece_type_at(int square) {
        uint64_t mask = 1ULL<<square;

        if (!(occupied & mask)) {
            return nullopt;  // Early return
        }
        else if (pawns & mask) {
            return PieceType::PAWN;
        }
        else if (knights & mask) {
            return PieceType::KNIGHT;
        }
        else if (bishops & mask) {
            return PieceType::BISHOP;
        }
        else if (rooks & mask) {
            return PieceType::ROOK;
        }
        else if (queens & mask) {
            return PieceType::QUEEN;
        }
        else {
            return PieceType::KING;
        }
    }

    optional<Color> color_at(int square) {
        uint64_t mask = 1ULL<<square;
        if (occupied_co[0] & mask) {
            return Color::WHITE;
        }
        else if (occupied_co[1] & mask) {
            return Color::BLACK;
        }
        else {
            return nullopt;
        }
    }

    string str();
};
