#ifndef MOVE_H_INCLUDED
#define MOVE_H_INCLUDED

#include "types.h"
    class Move
    {
        /*
        Represents a move from a square to a square and possibly the promotion
        piece type.

        Drops and null moves are supported.
        */

    public:
        Square from_square;
        /* The source square. */

        Square to_square;
        /* The target square. */

        std::optional<PieceType> promotion;
        /* The promotion piece type or ``std::nullopt``. */

        std::optional<PieceType> drop;
        /* The drop piece type or ``std::nullopt``. */

        Move(Square, Square, std::optional<PieceType> = std::nullopt, std::optional<PieceType> = std::nullopt);

        std::string uci() const;

        std::string xboard() const;

        operator bool() const;

        operator std::string() const;

        static Move from_uci(const std::string &);

        static Move null();
    };

    std::ostream &operator<<(std::ostream &, const Move &);
    int encode_raw_move(Move);
    Move decode_raw_move(int);
#endif // MOVE_H_INCLUDED
