#include "Move.h"
    Move::Move(Square from_square, Square to_square, std::optional<PieceType> promotion, std::optional<PieceType> drop) : from_square(from_square), to_square(to_square), promotion(promotion), drop(drop) {}

    std::string Move::uci() const
    {
        /*
        Gets a UCI string for the move.

        For example, a move from a7 to a8 would be ``a7a8`` or ``a7a8q``
        (if the latter is a promotion to a queen).

        The UCI representation of a null move is ``0000``.
        */
        if (this->drop)
        {
            return std::string(1, std::toupper(piece_symbol(*this->drop))) + "@" + SQUARE_NAMES[this->to_square];
        }
        else if (this->promotion)
        {
            return SQUARE_NAMES[this->from_square] + SQUARE_NAMES[this->to_square] + piece_symbol(*this->promotion);
        }
        else if (*this)
        {
            return SQUARE_NAMES[this->from_square] + SQUARE_NAMES[this->to_square];
        }
        else
        {
            return "0000";
        }
    }

    std::string Move::xboard() const
    {
        return *this ? this->uci() : "@@@@";
    }

    Move::operator bool() const
    {
        return bool(this->from_square || this->to_square || this->promotion || this->drop);
    }

    Move::operator std::string() const
    {
        return this->uci();
    }

    Move Move::from_uci(const std::string &uci)
    {
        /*
        Parses a UCI string.

        :throws: :exc:`std::invalid_argument` if the UCI string is invalid.
        */
        if (uci == "0000")
        {
            return Move::null();
        }
        else if (uci.length() == 4 && '@' == uci[1])
        {
            auto it = std::find(std::begin(PIECE_SYMBOLS), std::end(PIECE_SYMBOLS), std::tolower(uci[0]));
            if (it == std::end(PIECE_SYMBOLS))
            {
                throw std::invalid_argument("");
            }
            Square drop = std::distance(PIECE_SYMBOLS, it);
            auto it2 = std::find(std::begin(SQUARE_NAMES), std::end(SQUARE_NAMES), uci.substr(2));
            if (it2 == std::end(SQUARE_NAMES))
            {
                throw std::invalid_argument("");
            }
            Square square = std::distance(SQUARE_NAMES, it2);
            return Move(square, square, drop);
        }
        else if (4 <= uci.length() && uci.length() <= 5)
        {
            auto it = std::find(std::begin(SQUARE_NAMES), std::end(SQUARE_NAMES), uci.substr(0, 2));
            if (it == std::end(SQUARE_NAMES))
            {
                throw std::invalid_argument("");
            }
            Square from_square = std::distance(SQUARE_NAMES, it);
            auto it2 = std::find(std::begin(SQUARE_NAMES), std::end(SQUARE_NAMES), uci.substr(2, 4));
            if (it2 == std::end(SQUARE_NAMES))
            {
                throw std::invalid_argument("");
            }
            Square to_square = std::distance(SQUARE_NAMES, it2);
            std::optional<Square> promotion;
            if (uci.length() == 5)
            {
                auto it3 = std::find(std::begin(PIECE_SYMBOLS), std::end(PIECE_SYMBOLS), uci[4]);
                if (it3 == std::end(PIECE_SYMBOLS))
                {
                    throw std::invalid_argument("");
                }
                promotion = std::distance(PIECE_SYMBOLS, it3);
            }
            else
            {
                promotion = std::nullopt;
            }
            if (from_square == to_square)
            {
                throw std::invalid_argument("invalid uci (use 0000 for null moves): \"" + uci + "\"");
            }
            return Move(from_square, to_square, promotion);
        }
        else
        {
            throw std::invalid_argument("expected uci string to be of length 4 or 5: \"" + uci + "\"");
        }
    }

    Move Move::null()
    {
        /*
        Gets a null move.

        A null move just passes the turn to the other side (and possibly
        forfeits en passant capturing). Null moves evaluate to ``false`` in
        boolean contexts.

        >>> #include "chess.cpp"
        >>> #include <iostream>
        >>>
        >>> std::cout << bool(chess::Move::null());
        0
        */
        return Move(0, 0);
    }

    std::ostream &operator<<(std::ostream &os, const Move &move)
    {
        os << "Move::from_uci(\"" << move.uci() << "\")";
        return os;
    }
int encode_raw_move(Move move) {
    int raw_move = move.to_square;
    raw_move |= move.from_square << 6;
    raw_move |= (move.promotion ? move.promotion.value() - 1 : 0) << 12;
    return raw_move;
}
Move decode_raw_move(int raw_move) {
    int to_square = raw_move & 0x3f;
    int from_square = (raw_move >> 6) & 0x3f;
    int promotion_part = (raw_move >> 12) & 0x7;
    int promotion = promotion_part ? promotion_part + 1 : 0;
    if (promotion)return Move(from_square, to_square, promotion);
    else return Move(from_square, to_square, std::nullopt);
}
