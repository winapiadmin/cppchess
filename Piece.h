#ifndef PIECE_H_INCLUDED
#define PIECE_H_INCLUDED
#include "types.h"
    class Piece
    {
        /* A piece with type and color. */

    public:
        PieceType piece_type;
        /* The piece type. */

        Color color;
        /* The piece color. */

        Piece(PieceType, Color);

        Piece();

        char symbol() const;

        std::string unicode_symbol(bool = false) const;

        operator std::string() const;

        static Piece from_symbol(char);
        bool operator==(const Piece& p) const
        {return p.piece_type==this->piece_type&&p.color==this->color;}
    };

    std::ostream &operator<<(std::ostream &, const Piece &);



#endif // PIECE_H_INCLUDED
