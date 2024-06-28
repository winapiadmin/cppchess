#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <cmath>
#include <bitset>
#include <bit>
#include <vector>
#include <tuple>
#include <regex>
#include <functional>
#include <optional>
#include <sstream>
#include <stack>
#include <variant>
#include <array>
#include <iterator>
    typedef bool Color;
    const Color COLORS[] = {true, false}, WHITE = true, BLACK = false;
    const std::string COLOR_NAMES[] = {"black", "white"};

    typedef int PieceType;
    const PieceType PIECE_TYPES[] = {1, 2, 3, 4, 5, 6}, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;
    const std::optional<char> PIECE_SYMBOLS[] = {std::nullopt, 'p', 'n', 'b', 'r', 'q', 'k'};
    const std::optional<std::string> PIECE_NAMES[] = {std::nullopt, "pawn", "knight", "bishop", "rook", "queen", "king"};
    typedef std::string _EnPassantSpec;
    const char FILE_NAMES[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

    const char RANK_NAMES[] = {'1', '2', '3', '4', '5', '6', '7', '8'};

    #define STARTING_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    /* The FEN for the standard chess starting position. */

    #define STARTING_BOARD_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
    /* The board part of the FEN for the standard chess starting position. */

    typedef int Square;
    const Square SQUARES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}, A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7, A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15, A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23, A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31, A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39, A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47, A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55, A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63;

    const std::string SQUARE_NAMES[] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};


    inline char piece_symbol(PieceType piece_type)
    {
        return *PIECE_SYMBOLS[piece_type];
    }
    inline Square square(int file_index, int rank_index)
    {
        /* Gets a square number by file and rank index. */
        return rank_index * 8 + file_index;
    }

    inline int square_file(Square square)
    {
        /* Gets the file index of the square where ``0`` is the a-file. */
        return square & 7;
    }

    inline int square_rank(Square square)
    {
        /* Gets the rank index of the square where ``0`` is the first rank. */
        return square >> 3;
    }

    inline int square_distance(Square a, Square b)
    {
        /*
        Gets the distance (i.e., the number of king steps) from square *a* to *b*.
        */
        return std::max(abs(square_file(a) - square_file(b)), abs(square_rank(a) - square_rank(b)));
    }

    inline Square square_mirror(Square square)
    {
        /* Mirrors the square vertically. */
        return square ^ 0x38;
    }


    const Square SQUARES_180[] = {square_mirror(0), square_mirror(1), square_mirror(2), square_mirror(3), square_mirror(4), square_mirror(5), square_mirror(6), square_mirror(7), square_mirror(8), square_mirror(9), square_mirror(10), square_mirror(11), square_mirror(12), square_mirror(13), square_mirror(14), square_mirror(15), square_mirror(16), square_mirror(17), square_mirror(18), square_mirror(19), square_mirror(20), square_mirror(21), square_mirror(22), square_mirror(23), square_mirror(24), square_mirror(25), square_mirror(26), square_mirror(27), square_mirror(28), square_mirror(29), square_mirror(30), square_mirror(31), square_mirror(32), square_mirror(33), square_mirror(34), square_mirror(35), square_mirror(36), square_mirror(37), square_mirror(38), square_mirror(39), square_mirror(40), square_mirror(41), square_mirror(42), square_mirror(43), square_mirror(44), square_mirror(45), square_mirror(46), square_mirror(47), square_mirror(48), square_mirror(49), square_mirror(50), square_mirror(51), square_mirror(52), square_mirror(53), square_mirror(54), square_mirror(55), square_mirror(56), square_mirror(57), square_mirror(58), square_mirror(59), square_mirror(60), square_mirror(61), square_mirror(62), square_mirror(63)};

    typedef unsigned long long Bitboard;
    #define BB_EMPTY  0
    #define BB_ALL  0xffff'ffff'ffff'ffffULL

    const Bitboard BB_SQUARES[] = {
        1ULL << 0,
        1ULL << 1,
        1ULL << 2,
        1ULL << 3,
        1ULL << 4,
        1ULL << 5,
        1ULL << 6,
        1ULL << 7,
        1ULL << 8,
        1ULL << 9,
        1ULL << 10,
        1ULL << 11,
        1ULL << 12,
        1ULL << 13,
        1ULL << 14,
        1ULL << 15,
        1ULL << 16,
        1ULL << 17,
        1ULL << 18,
        1ULL << 19,
        1ULL << 20,
        1ULL << 21,
        1ULL << 22,
        1ULL << 23,
        1ULL << 24,
        1ULL << 25,
        1ULL << 26,
        1ULL << 27,
        1ULL << 28,
        1ULL << 29,
        1ULL << 30,
        1ULL << 31,
        1ULL << 32,
        1ULL << 33,
        1ULL << 34,
        1ULL << 35,
        1ULL << 36,
        1ULL << 37,
        1ULL << 38,
        1ULL << 39,
        1ULL << 40,
        1ULL << 41,
        1ULL << 42,
        1ULL << 43,
        1ULL << 44,
        1ULL << 45,
        1ULL << 46,
        1ULL << 47,
        1ULL << 48,
        1ULL << 49,
        1ULL << 50,
        1ULL << 51,
        1ULL << 52,
        1ULL << 53,
        1ULL << 54,
        1ULL << 55,
        1ULL << 56,
        1ULL << 57,
        1ULL << 58,
        1ULL << 59,
        1ULL << 60,
        1ULL << 61,
        1ULL << 62,
        1ULL << 63,
    },
                   BB_A1 = 1ULL << 0, BB_B1 = 1ULL << 1, BB_C1 = 1ULL << 2, BB_D1 = 1ULL << 3, BB_E1 = 1ULL << 4, BB_F1 = 1ULL << 5, BB_G1 = 1ULL << 6, BB_H1 = 1ULL << 7, BB_A2 = 1ULL << 8, BB_B2 = 1ULL << 9, BB_C2 = 1ULL << 10, BB_D2 = 1ULL << 11, BB_E2 = 1ULL << 12, BB_F2 = 1ULL << 13, BB_G2 = 1ULL << 14, BB_H2 = 1ULL << 15, BB_A3 = 1ULL << 16, BB_B3 = 1ULL << 17, BB_C3 = 1ULL << 18, BB_D3 = 1ULL << 19, BB_E3 = 1ULL << 20, BB_F3 = 1ULL << 21, BB_G3 = 1ULL << 22, BB_H3 = 1ULL << 23, BB_A4 = 1ULL << 24, BB_B4 = 1ULL << 25, BB_C4 = 1ULL << 26, BB_D4 = 1ULL << 27, BB_E4 = 1ULL << 28, BB_F4 = 1ULL << 29, BB_G4 = 1ULL << 30, BB_H4 = 1ULL << 31, BB_A5 = 1ULL << 32, BB_B5 = 1ULL << 33, BB_C5 = 1ULL << 34, BB_D5 = 1ULL << 35, BB_E5 = 1ULL << 36, BB_F5 = 1ULL << 37, BB_G5 = 1ULL << 38, BB_H5 = 1ULL << 39, BB_A6 = 1ULL << 40, BB_B6 = 1ULL << 41, BB_C6 = 1ULL << 42, BB_D6 = 1ULL << 43, BB_E6 = 1ULL << 44, BB_F6 = 1ULL << 45, BB_G6 = 1ULL << 46, BB_H6 = 1ULL << 47, BB_A7 = 1ULL << 48, BB_B7 = 1ULL << 49, BB_C7 = 1ULL << 50, BB_D7 = 1ULL << 51, BB_E7 = 1ULL << 52, BB_F7 = 1ULL << 53, BB_G7 = 1ULL << 54, BB_H7 = 1ULL << 55, BB_A8 = 1ULL << 56, BB_B8 = 1ULL << 57, BB_C8 = 1ULL << 58, BB_D8 = 1ULL << 59, BB_E8 = 1ULL << 60, BB_F8 = 1ULL << 61, BB_G8 = 1ULL << 62, BB_H8 = 1ULL << 63;

    #define BB_CORNERS  BB_A1 | BB_H1 | BB_A8 | BB_H8
    #define BB_CENTER  BB_D4 | BB_E4 | BB_D5 | BB_E5

    #define BB_LIGHT_SQUARES  0x55aa'55aa'55aa'55aa
    #define BB_DARK_SQUARES  0xaa55'aa55'aa55'aa55

    const Bitboard BB_FILES[] = {
        0x0101'0101'0101'0101ULL << 0,
        0x0101'0101'0101'0101ULL << 1,
        0x0101'0101'0101'0101ULL << 2,
        0x0101'0101'0101'0101ULL << 3,
        0x0101'0101'0101'0101ULL << 4,
        0x0101'0101'0101'0101ULL << 5,
        0x0101'0101'0101'0101ULL << 6,
        0x0101'0101'0101'0101ULL << 7,
    },
                   BB_FILE_A = 0x0101'0101'0101'0101ULL << 0, BB_FILE_B = 0x0101'0101'0101'0101ULL << 1, BB_FILE_C = 0x0101'0101'0101'0101ULL << 2, BB_FILE_D = 0x0101'0101'0101'0101ULL << 3, BB_FILE_E = 0x0101'0101'0101'0101ULL << 4, BB_FILE_F = 0x0101'0101'0101'0101ULL << 5, BB_FILE_G = 0x0101'0101'0101'0101ULL << 6, BB_FILE_H = 0x0101'0101'0101'0101ULL << 7;

    const Bitboard BB_RANKS[] = {
        0xffULL << (8 * 0),
        0xffULL << (8 * 1),
        0xffULL << (8 * 2),
        0xffULL << (8 * 3),
        0xffULL << (8 * 4),
        0xffULL << (8 * 5),
        0xffULL << (8 * 6),
        0xffULL << (8 * 7),
    },
                   BB_RANK_1 = 0xffULL << (8 * 0), BB_RANK_2 = 0xffULL << (8 * 1), BB_RANK_3 = 0xffULL << (8 * 2), BB_RANK_4 = 0xffULL << (8 * 3), BB_RANK_5 = 0xffULL << (8 * 4), BB_RANK_6 = 0xffULL << (8 * 5), BB_RANK_7 = 0xffULL << (8 * 6), BB_RANK_8 = 0xffULL << (8 * 7);

    #define BB_BACKRANKS  BB_RANK_1 | BB_RANK_8

    #define lsb(bb) std::numeric_limits<Bitboard>::digits - std::countl_zero(bb & -bb) - 1
    inline std::vector<Square> scan_forward(Bitboard bb)
    {
        std::vector<Square> iter;
        while (bb)
        {
            Bitboard r = bb & -bb;
            iter.push_back(std::numeric_limits<Bitboard>::digits - std::countl_zero(r) - 1);
            bb ^= r;
        }
        return iter;
    }
    #define msb(bb)std::numeric_limits<Bitboard>::digits - std::countl_zero(bb) - 1

    inline std::vector<Square> scan_reversed(Bitboard bb)
    {
        std::vector<Square> iter;
        while (bb)
        {
            Square r = std::numeric_limits<Bitboard>::digits - std::countl_zero(bb) - 1;
            iter.push_back(r);
            bb ^= BB_SQUARES[r];
        }
        return iter;
    }

    inline Bitboard flip_vertical(Bitboard bb)
    {
        // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipVertically
        bb = ((bb >> 8) & 0x00ff'00ff'00ff'00ff) | ((bb & 0x00ff'00ff'00ff'00ff) << 8);
        bb = ((bb >> 16) & 0x0000'ffff'0000'ffff) | ((bb & 0x0000'ffff'0000'ffff) << 16);
        bb = (bb >> 32) | ((bb & 0x0000'0000'ffff'ffff) << 32);
        return bb;
    }

    inline Bitboard flip_horizontal(Bitboard bb)
    {
        // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#MirrorHorizontally
        bb = ((bb >> 1) & 0x5555'5555'5555'5555) | ((bb & 0x5555'5555'5555'5555) << 1);
        bb = ((bb >> 2) & 0x3333'3333'3333'3333) | ((bb & 0x3333'3333'3333'3333) << 2);
        bb = ((bb >> 4) & 0x0f0f'0f0f'0f0f'0f0f) | ((bb & 0x0f0f'0f0f'0f0f'0f0f) << 4);
        return bb;
    }

    inline Bitboard flip_diagonal(Bitboard bb)
    {
        // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheDiagonal
        Bitboard t = (bb ^ (bb << 28)) & 0x0f0f'0f0f'0000'0000;
        bb = bb ^ (t ^ (t >> 28));
        t = (bb ^ (bb << 14)) & 0x3333'0000'3333'0000;
        bb = bb ^ (t ^ (t >> 14));
        t = (bb ^ (bb << 7)) & 0x5500'5500'5500'5500;
        bb = bb ^ (t ^ (t >> 7));
        return bb;
    }

    inline Bitboard flip_anti_diagonal(Bitboard bb)
    {
        // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheAntidiagonal
        Bitboard t = bb ^ (bb << 36);
        bb = bb ^ ((t ^ (bb >> 36)) & 0xf0f0'f0f0'0f0f'0f0f);
        t = (bb ^ (bb << 18)) & 0xcccc'0000'cccc'0000;
        bb = bb ^ (t ^ (t >> 18));
        t = (bb ^ (bb << 9)) & 0xaa00'aa00'aa00'aa00;
        bb = bb ^ (t ^ (t >> 9));
        return bb;
    }
#define shift_down(b) b>>8
#define shift_up(b) (b << 8) & BB_ALL

    #define STATUS_VALID  0
    #define STATUS_NO_WHITE_KING  1 << 0
    #define STATUS_NO_BLACK_KING  1<<1
    #define STATUS_TOO_MANY_KINGS  1<<2
    #define STATUS_TOO_MANY_WHITE_PAWNS  1<<3
    #define STATUS_TOO_MANY_BLACK_PAWNS  1<<4
    #define STATUS_PAWNS_ON_BACKRANK 1<<5
    #define STATUS_TOO_MANY_WHITE_PIECES  1<<6
    #define STATUS_TOO_MANY_BLACK_PIECES  1<<7
    #define STATUS_BAD_CASTLING_RIGHTS  1<<8
    #define STATUS_INVALID_EP_SQUARE  1<<9
    #define STATUS_OPPOSITE_CHECK  1<<10
    #define STATUS_EMPTY  1<<11
    #define STATUS_RACE_CHECK  1<<12
    #define STATUS_RACE_OVER  1<<13
    #define STATUS_RACE_MATERIAL  1<<14
    #define STATUS_TOO_MANY_CHECKERS  1<<15
    #define STATUS_IMPOSSIBLE_CHECK  1<<16
    const std::regex SAN_REGEX(R"(^([NBKRQ])?([a-h])?([1-8])?[\-x]?([a-h][1-8])(=?[nbrqkNBRQK])?[\+#]?$)");

    const std::regex FEN_CASTLING_REGEX(R"(^(?:-|[KQABCDEFGH]{0,2}[kqabcdefgh]{0,2})$)");
    #define popcount(bb) __builtin_popcount(bb)
#endif // TYPES_H_INCLUDED
