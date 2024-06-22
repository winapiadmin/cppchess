#include "BaseBoard.h"

BaseBoard::BaseBoard(const std::optional<std::string> &board_fen): occupied_co{BB_EMPTY, BB_EMPTY}
{
    if (board_fen == std::nullopt)
    {
        this->_clear_board();
    }
    else if (*board_fen == STARTING_BOARD_FEN)
    {
        this->_reset_board();
    }
    else
    {
        this->_set_board_fen(*board_fen);
    }
}

Bitboard BaseBoard::_attackers_mask(Color color, Square square, Bitboard occupied) const
{
    Bitboard rank_pieces = BB_RANK_MASKS[square] & occupied;
    Bitboard file_pieces = BB_FILE_MASKS[square] & occupied;
    Bitboard diag_pieces = BB_DIAG_MASKS[square] & occupied;

    Bitboard queens_and_rooks = this->queens | this->rooks;
    Bitboard queens_and_bishops = this->queens | this->bishops;

    Bitboard attackers = ((BB_KING_ATTACKS[square] & this->kings) |
                          (BB_KNIGHT_ATTACKS[square] & this->knights) |
                          (BB_RANK_ATTACKS[square].at(rank_pieces) & queens_and_rooks) |
                          (BB_FILE_ATTACKS[square].at(file_pieces) & queens_and_rooks) |
                          (BB_DIAG_ATTACKS[square].at(diag_pieces) & queens_and_bishops) |
                          (BB_PAWN_ATTACKS[!color][square] & this->pawns));

    return attackers & this->occupied_co[color];
}
Bitboard BaseBoard::attackers_mask(Color color, Square square) const
{
    return this->_attackers_mask(color, square, this->occupied);
}


    void BaseBoard::reset_board()
    {
        /* Resets pieces to the starting position. */
        this->_reset_board();
    }

    void BaseBoard::clear_board()
    {
        /* Clears the board. */
        this->_clear_board();
    }

    Bitboard BaseBoard::pin_mask(Color color, Square square) const
    {
        /*
        Detects an absolute pin (and its direction) of the given square to
        the king of the given color.

        >>> #include "chess.cpp"
        >>> #include <iostream>
        >>>
        >>> chess::Board board = chess::Board("rnb1k2r/ppp2ppp/5n2/3q4/1b1P4/2N5/PP3PPP/R1BQKBNR w KQkq - 3 7");
        >>> std::cout << board.is_pinned(chess::WHITE, chess::C3);
        1
        >>> chess::SquareSet direction = board.pin(chess::WHITE, chess::C3);
        >>> std::cout << direction;
        SquareSet(0x0000'0001'0204'0810)
        >>> std::cout << std::string(direction);
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        1 . . . . . . .
        . 1 . . . . . .
        . . 1 . . . . .
        . . . 1 . . . .
        . . . . 1 . . .

        Returns a :class:`set of squares <chess::SquareSet>` that mask the rank,
        file or diagonal of the pin. If there is no pin, then a mask of the
        entire board is returned.
        */
        std::optional<Square> king = this->king(color);
        if (king == std::nullopt)
            return BB_ALL;

        Bitboard square_mask = BB_SQUARES[square];

        for (auto [attacks, sliders] : {std::make_tuple(BB_FILE_ATTACKS, this->rooks | this->queens),
                                        std::make_tuple(BB_RANK_ATTACKS, this->rooks | this->queens),
                                        std::make_tuple(BB_DIAG_ATTACKS, this->bishops | this->queens)})
        {
            Bitboard rays = attacks[*king].at(0);
            if (rays & square_mask)
            {
                Bitboard snipers = rays & sliders & this->occupied_co[!color];
                for (Square sniper : scan_reversed(snipers))
                {
                    if ((between(sniper, *king) & (this->occupied | square_mask)) == square_mask)
                    {
                        return ray(*king, sniper);
                    }
                }

                break;
            }
        }

        return BB_ALL;
    }

    bool BaseBoard::is_pinned(Color color, Square square) const
    {
        /*
        Detects if the given square is pinned to the king of the given color.
        */
        return this->pin_mask(color, square) != BB_ALL;
    }

    std::optional<Piece> BaseBoard::remove_piece_at(Square square)
    {
        /*
        Removes the piece from the given square. Returns the
        :class:`~chess::Piece` or ``std::nullopt`` if the square was already empty.
        */
        Color color = bool(this->occupied_co[WHITE] & BB_SQUARES[square]);
        std::optional<PieceType> piece_type = this->_remove_piece_at(square);
        return piece_type ? std::optional(Piece(*piece_type, color)) : std::nullopt;
    }

    void BaseBoard::set_piece_at(Square square, const std::optional<Piece> &piece, bool promoted)
    {
        /*
        Sets a piece at the given square.

        An existing piece is replaced. Setting *piece* to ``std::nullopt`` is
        equivalent to :func:`~chess::Board::remove_piece_at()`.
        */
        if (piece == std::nullopt)
        {
            this->_remove_piece_at(square);
        }
        else
        {
            this->_set_piece_at(square, piece->piece_type, piece->color, promoted);
        }
    }

    std::string BaseBoard::board_fen(std::optional<bool> promoted) const
    {
        /*
        Gets the board FEN (e.g.,
        ``rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR``).
        */
        std::vector<char> builder;
        int empty = 0;

        for (Square square : SQUARES_180)
        {
            std::optional<Piece> piece = this->piece_at(square);

            if (!piece)
            {
                ++empty;
            }
            else
            {
                if (empty)
                {
                    builder.push_back(std::to_string(empty)[0]);
                    empty = 0;
                }
                builder.push_back(piece->symbol());
                if (promoted && BB_SQUARES[square] & this->promoted)
                {
                    builder.push_back('~');
                }
            }

            if (BB_SQUARES[square] & BB_FILE_H)
            {
                if (empty)
                {
                    builder.push_back(std::to_string(empty)[0]);
                    empty = 0;
                }

                if (square != H1)
                {
                    builder.push_back('/');
                }
            }
        }

        return std::string(std::begin(builder), std::end(builder));
    }

    void BaseBoard::set_board_fen(const std::string &fen)
    {
        /*
        Parses *fen* and sets up the board, where *fen* is the board part of
        a FEN.

        :throws: :exc:`std::invalid_argument` if syntactically invalid.
        */
        this->_set_board_fen(fen);
    }

    void BaseBoard::_reset_board()
    {
        this->pawns = BB_RANK_2 | BB_RANK_7;
        this->knights = BB_B1 | BB_G1 | BB_B8 | BB_G8;
        this->bishops = BB_C1 | BB_F1 | BB_C8 | BB_F8;
        this->rooks = BB_CORNERS;
        this->queens = BB_D1 | BB_D8;
        this->kings = BB_E1 | BB_E8;

        this->promoted = BB_EMPTY;

        this->occupied_co[WHITE] = BB_RANK_1 | BB_RANK_2;
        this->occupied_co[BLACK] = BB_RANK_7 | BB_RANK_8;
        this->occupied = BB_RANK_1 | BB_RANK_2 | BB_RANK_7 | BB_RANK_8;
    }

    void BaseBoard::_clear_board()
    {
        this->pawns = BB_EMPTY;
        this->knights = BB_EMPTY;
        this->bishops = BB_EMPTY;
        this->rooks = BB_EMPTY;
        this->queens = BB_EMPTY;
        this->kings = BB_EMPTY;

        this->promoted = BB_EMPTY;

        this->occupied_co[WHITE] = BB_EMPTY;
        this->occupied_co[BLACK] = BB_EMPTY;
        this->occupied = BB_EMPTY;
    }

    Bitboard BaseBoard::attacks_mask(Square square) const
    {
        Bitboard bb_square = BB_SQUARES[square];

        if (bb_square & this->pawns)
        {
            Color color = bool(bb_square & this->occupied_co[WHITE]);
            return BB_PAWN_ATTACKS[color][square];
        }
        else if (bb_square & this->knights)
        {
            return BB_KNIGHT_ATTACKS[square];
        }
        else if (bb_square & this->kings)
        {
            return BB_KING_ATTACKS[square];
        }
        else
        {
            Bitboard attacks = 0;
            if (bb_square & this->bishops || bb_square & this->queens)
            {
                attacks = BB_DIAG_ATTACKS[square].at(BB_DIAG_MASKS[square] & this->occupied);
            }
            if (bb_square & this->rooks || bb_square & this->queens)
            {
                attacks |= (BB_RANK_ATTACKS[square].at(BB_RANK_MASKS[square] & this->occupied) |
                            BB_FILE_ATTACKS[square].at(BB_FILE_MASKS[square] & this->occupied));
            }
            return attacks;
        }
    }
    BaseBoard BaseBoard::copy() const
    {
        /* Creates a copy of the board. */
        BaseBoard board = BaseBoard(std::nullopt);

        board.pawns = this->pawns;
        board.knights = this->knights;
        board.bishops = this->bishops;
        board.rooks = this->rooks;
        board.queens = this->queens;
        board.kings = this->kings;

        board.occupied_co[WHITE] = this->occupied_co[WHITE];
        board.occupied_co[BLACK] = this->occupied_co[BLACK];
        board.occupied = this->occupied;
        board.promoted = this->promoted;

        return board;
    }

    bool BaseBoard::is_attacked_by(Color color, Square square) const
    {
        /*
        Checks if the given side attacks the given square.

        Pinned pieces still count as attackers. Pawns that can be captured
        en passant are **not** considered attacked.
        */
        return bool(this->attackers_mask(color, square));
    }

    std::optional<PieceType> BaseBoard::_remove_piece_at(Square square)
    {
        std::optional<PieceType> piece_type = this->piece_type_at(square);
        Bitboard mask = BB_SQUARES[square];

        if (*piece_type == PAWN)
        {
            this->pawns ^= mask;
        }
        else if (*piece_type == KNIGHT)
        {
            this->knights ^= mask;
        }
        else if (*piece_type == BISHOP)
        {
            this->bishops ^= mask;
        }
        else if (*piece_type == ROOK)
        {
            this->rooks ^= mask;
        }
        else if (*piece_type == QUEEN)
        {
            this->queens ^= mask;
        }
        else if (*piece_type == KING)
        {
            this->kings ^= mask;
        }
        else
        {
            return std::nullopt;
        }

        this->occupied ^= mask;
        this->occupied_co[WHITE] &= ~mask;
        this->occupied_co[BLACK] &= ~mask;

        this->promoted &= ~mask;

        return piece_type;
    }

    void BaseBoard::_set_piece_at(Square square, PieceType piece_type, Color color, bool promoted)
    {
        this->_remove_piece_at(square);

        Bitboard mask = BB_SQUARES[square];

        if (piece_type == PAWN)
        {
            this->pawns |= mask;
        }
        else if (piece_type == KNIGHT)
        {
            this->knights |= mask;
        }
        else if (piece_type == BISHOP)
        {
            this->bishops |= mask;
        }
        else if (piece_type == ROOK)
        {
            this->rooks |= mask;
        }
        else if (piece_type == QUEEN)
        {
            this->queens |= mask;
        }
        else if (piece_type == KING)
        {
            this->kings |= mask;
        }
        else
        {
            return;
        }

        this->occupied ^= mask;
        this->occupied_co[color] ^= mask;

        if (promoted)
        {
            this->promoted ^= mask;
        }
    }

    void BaseBoard::_set_board_fen(std::string fen)
    {
        // Compatibility with set_fen().
        auto it = begin(fen);
        auto it2 = rbegin(fen);
        while (isspace(*it))
        {
            ++it;
        }
        while (isspace(*it2))
        {
            ++it2;
        }
        fen = std::string(it, it2.base());
        if (fen.find(' ') != std::string::npos)
        {
            throw std::invalid_argument("expected position part of fen, got multiple parts: \"" + fen + "\"");
        }

        // Ensure the FEN is valid.
        std::vector<std::string> rows;
        for (size_t i = 0, dist = 0; i < fen.length(); ++i, ++dist)
        {
            if (fen[i] == '/')
            {
                rows.push_back(fen.substr(i++ - dist, dist));
                dist = 0;
            }
            else if (i == fen.length() - 1)
            {
                rows.push_back(fen.substr(i - dist));
            }
        }
        if (rows.size() != 8)
        {
            throw std::invalid_argument("expected 8 rows in position part of fen: \"" + fen + "\"");
        }

        // Validate each row.
        for (const std::string &row : rows)
        {
            int field_sum = 0;
            bool previous_was_digit = false;
            bool previous_was_piece = false;

            for (char c : row)
            {
                if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8')
                {
                    if (previous_was_digit)
                    {
                        throw std::invalid_argument("two subsequent digits in position part of fen: \"" + fen + "\"");
                    }
                    field_sum += c - '0';
                    previous_was_digit = true;
                    previous_was_piece = false;
                }
                else if (c == '~')
                {
                    if (!previous_was_piece)
                    {
                        throw std::invalid_argument("'~' not after piece in position part of fen: \"" + fen + "\"");
                    }
                    previous_was_digit = false;
                    previous_was_piece = false;
                }
                else if (std::find(std::begin(PIECE_SYMBOLS), std::end(PIECE_SYMBOLS), std::tolower(c)) != std::end(PIECE_SYMBOLS))
                {
                    ++field_sum;
                    previous_was_digit = false;
                    previous_was_piece = true;
                }
                else
                {
                    throw std::invalid_argument("invalid character in position part of fen: \"" + fen + "\"");
                }
            }

            if (field_sum != 8)
            {
                throw std::invalid_argument("expected 8 columns per row in position part of fen: \"" + fen + "\"");
            }
        }

        // Clear the board.
        this->_clear_board();

        // Put pieces on the board.
        int square_index = 0;
        for (char c : fen)
        {
            if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8')
            {
                square_index += c - '0';
            }
            else if (std::find(std::begin(PIECE_SYMBOLS), std::end(PIECE_SYMBOLS), std::tolower(c)) != std::end(PIECE_SYMBOLS))
            {
                Piece piece = Piece::from_symbol(c);
                this->_set_piece_at(SQUARES_180[square_index], piece.piece_type, piece.color);
                ++square_index;
            }
            else if (c == '~')
            {
                this->promoted |= BB_SQUARES[SQUARES_180[square_index - 1]];
            }
        }
    }

    std::optional<Square> BaseBoard::king(Color color) const
    {
        /*
        Finds the king square of the given side. Returns ``std::nullopt`` if there
        is no king of that color.

        In variants with king promotions, only non-promoted kings are
        considered.
        */
        Bitboard king_mask = this->occupied_co[color] & this->kings & ~this->promoted;
        return king_mask ? std::optional(msb(king_mask)) : std::nullopt;
    }

        std::optional<Piece> BaseBoard::piece_at(Square square) const
    {
        /* Gets the :class:`piece <chess::Piece>` at the given square. */
        std::optional<PieceType> piece_type = this->piece_type_at(square);
        if (piece_type)
        {
            Bitboard mask = BB_SQUARES[square];
            Color color = bool(this->occupied_co[WHITE] & mask);
            return Piece(*piece_type, color);
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<PieceType> BaseBoard::piece_type_at(Square square) const
    {
        /* Gets the piece type at the given square. */
        Bitboard mask = BB_SQUARES[square];

        if (!(this->occupied & mask))
        {
            return std::nullopt; // Early return
        }
        else if (this->pawns & mask)
        {
            return PAWN;
        }
        else if (this->knights & mask)
        {
            return KNIGHT;
        }
        else if (this->bishops & mask)
        {
            return BISHOP;
        }
        else if (this->rooks & mask)
        {
            return ROOK;
        }
        else if (this->queens & mask)
        {
            return QUEEN;
        }
        else
        {
            return KING;
        }
    }

    std::optional<Color> BaseBoard::color_at(Square square) const
    {
        /* Gets the color of the piece at the given square. */
        Bitboard mask = BB_SQUARES[square];
        if (this->occupied_co[WHITE] & mask)
        {
            return WHITE;
        }
        else if (this->occupied_co[BLACK] & mask)
        {
            return BLACK;
        }
        else
        {
            return std::nullopt;
        }
    }

    Bitboard BaseBoard::pieces_mask(PieceType piece_type, Color color) const
    {
        Bitboard bb;
        if (piece_type == PAWN)
        {
            bb = this->pawns;
        }
        else if (piece_type == KNIGHT)
        {
            bb = this->knights;
        }
        else if (piece_type == BISHOP)
        {
            bb = this->bishops;
        }
        else if (piece_type == ROOK)
        {
            bb = this->rooks;
        }
        else if (piece_type == QUEEN)
        {
            bb = this->queens;
        }
        else if (piece_type == KING)
        {
            bb = this->kings;
        }
        else
        {
            throw std::runtime_error("expected PieceType, got \"" + std::to_string(piece_type) + "\"");
        }

        return bb & this->occupied_co[color];
    }
