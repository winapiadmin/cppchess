#ifndef BASEBOARD_H
#define BASEBOARD_H
#include "movegen.h"
#include "Piece.h"
#include "Move.h"
class BaseBoard
{
    public:
        /** Default constructor */
        BaseBoard(const std::optional<std::string> & = STARTING_BOARD_FEN);

        unsigned long long occupied=0; //!< Member variable "occupied;"
        unsigned long long occupied_co[2]; //!< Member variable "occupied_co[2];"
        unsigned long long bishops=0; //!< Member variable "bishops;"
        unsigned long long rooks=0; //!< Member variable "rooks;"
        unsigned long long queens=0; //!< Member variable "queens;"
        unsigned long long pawns=0; //!< Member variable "pawns;"
        unsigned long long knights=0; //!< Member variable "knights;"
        unsigned long long kings=0; //!< Member variable "kings;"
        unsigned long long promoted=0;
        //Bitboard checkers_mask() const;
        Bitboard attackers_mask(Color, Square) const;
        void reset_board();

        void clear_board();
        Bitboard pin_mask(Color, Square) const;

        bool is_pinned(Color, Square) const;

        std::optional<Piece> remove_piece_at(Square);

        void set_piece_at(Square, const std::optional<Piece> &, bool = false);

        std::string board_fen(std::optional<bool> = false) const;

        Bitboard attacks_mask(Square) const;
        bool is_attacked_by(Color, Square) const;
        void set_board_fen(const std::string &);
        std::optional<Square> king(Color) const;
        std::optional<Piece> piece_at(Square) const;

        std::optional<PieceType> piece_type_at(Square) const;

        std::optional<Color> color_at(Square) const;
        BaseBoard copy() const;
        Bitboard pieces_mask(PieceType, Color) const;
    protected:
        void _reset_board();

        void _clear_board();

        Bitboard _attackers_mask(Color, Square, Bitboard) const;

        std::optional<PieceType> _remove_piece_at(Square);

        void _set_piece_at(Square, PieceType, Color, bool = false);

        void _set_board_fen(std::string);

};

#endif // BASEBOARD_H
