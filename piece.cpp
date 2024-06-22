#include "Piece.h"
    Piece::Piece(PieceType piece_type, Color color) : piece_type(piece_type), color(color) {}

    char Piece::symbol() const
    {
        /*
        Gets the symbol ``P``, ``N``, ``B``, ``R``, ``Q`` or ``K`` for white
        pieces or the lower-case variants for the black pieces.
        */
        char symbol = piece_symbol(this->piece_type);
        return this->color ? std::toupper(symbol) : symbol;
    }


    Piece::operator std::string() const
    {
        return std::string(1, this->symbol());
    }

    Piece Piece::from_symbol(char symbol)
    {
        /*
        Creates a :class:`~chess::Piece` instance from a piece symbol.

        :throws: :exc:`std::invalid_argument` if the symbol is invalid.
        */
        auto it = std::find(std::begin(PIECE_SYMBOLS), std::end(PIECE_SYMBOLS), std::tolower(symbol));
        if (it == std::end(PIECE_SYMBOLS))
        {
            throw std::invalid_argument("symbol is invalid");
        }
        return Piece(std::distance(PIECE_SYMBOLS, it), std::isupper(symbol));
    }

    std::ostream &operator<<(std::ostream &os, const Piece &piece)
    {
        os << "Piece::from_symbol('" << piece.symbol() << "')";
        return os;
    }
