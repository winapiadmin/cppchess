#include "Board.h"

    _BoardState::_BoardState(const Board &board)
    {
        this->pawns = board.pawns;
        this->knights = board.knights;
        this->bishops = board.bishops;
        this->rooks = board.rooks;
        this->queens = board.queens;
        this->kings = board.kings;

        this->occupied_w = board.occupied_co[WHITE];
        this->occupied_b = board.occupied_co[BLACK];
        this->occupied = board.occupied;

        this->promoted = board.promoted;

        this->turn = board.turn;
        this->castling_rights = board.castling_rights;
        this->ep_square = board.ep_square;
        this->halfmove_clock = board.halfmove_clock;
        this->fullmove_number = board.fullmove_number;
    }

    void _BoardState::restore(Board &board) const
    {
        board.pawns = this->pawns;
        board.knights = this->knights;
        board.bishops = this->bishops;
        board.rooks = this->rooks;
        board.queens = this->queens;
        board.kings = this->kings;

        board.occupied_co[WHITE] = this->occupied_w;
        board.occupied_co[BLACK] = this->occupied_b;
        board.occupied = this->occupied;

        board.promoted = this->promoted;

        board.turn = this->turn;
        board.castling_rights = this->castling_rights;
        board.ep_square = this->ep_square;
        board.halfmove_clock = this->halfmove_clock;
        board.fullmove_number = this->fullmove_number;
    }


    std::optional<std::string> Board::uci_variant = "chess";
    std::optional<std::string> Board::xboard_variant = "normal";
    std::string Board::starting_fen = STARTING_FEN;
    Board::Board(const std::optional<std::string> &fen, bool chess960) : BaseBoard(std::nullopt)
    {
        this->chess960 = chess960;

        this->ep_square = std::nullopt;

        if (fen == std::nullopt)
        {
            this->clear();
        }
        else if (*fen == Board::starting_fen)
        {
            this->reset();
        }
        else
        {
            this->set_fen(*fen);
        }
    }

    void Board::reset()
    {
        /* Restores the starting position. */
        this->turn = WHITE;
        this->castling_rights = BB_CORNERS;
        this->ep_square = std::nullopt;
        this->halfmove_clock = 0;
        this->fullmove_number = 1;

        this->reset_board();
    }

    void Board::reset_board()
    {
        /*
        Resets only pieces to the starting position. Use
        :func:`~Board::reset()` to fully restore the starting position
        (including turn, castling rights, etc.).
        */
        BaseBoard::reset_board();
        this->clear_stack();
    }

    void Board::clear()
    {
        /*
        Clears the board.

        Resets move stack and move counters. The side to move is white. There
        are no rooks or kings, so castling rights are removed.

        In order to be in a valid :func:`~Board::status()`, at least kings
        need to be put on the board.
        */
        this->turn = WHITE;
        this->castling_rights = BB_EMPTY;
        this->ep_square = std::nullopt;
        this->halfmove_clock = 0;
        this->fullmove_number = 1;

        this->clear_board();
    }

    void Board::clear_board()
    {
        BaseBoard::clear_board();
        this->clear_stack();
    }

    void Board::clear_stack()
    {
        /* Clears the move stack. */
        this->move_stack.clear();
        this->_stack.clear();
    }

    Board Board::root() const
    {
        /* Returns a copy of the root position. */
        if (!this->_stack.empty())
        {
            Board board = Board(std::nullopt, this->chess960);
            this->_stack.front().restore(board);
            return board;
        }
        else
        {
            return this->copy(false);
        }
    }

    int Board::ply() const
    {
        /*
        Returns the number of half-moves since the start of the game, as
        indicated by :data:`~Board::fullmove_number` and
        :data:`~Board::turn`.

        If moves have been pushed from the beginning, this is usually equal to
        ``board.move_stack.size()``. But note that a board can be set up with
        arbitrary starting positions, and the stack can be cleared.
        */
        return 2 * (this->fullmove_number - 1) + (this->turn == BLACK);
    }

    std::optional<Piece> Board::remove_piece_at(Square square)
    {
        std::optional<Piece> piece = BaseBoard::remove_piece_at(square);
        this->clear_stack();
        return piece;
    }

    void Board::set_piece_at(Square square, const std::optional<Piece> &piece, bool promoted)
    {
        BaseBoard::set_piece_at(square, piece, promoted);
        this->clear_stack();
    }

    std::vector<Move> Board::generate_pseudo_legal_moves(Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;
        Bitboard our_pieces = this->occupied_co[this->turn];

        // Generate piece moves.
        Bitboard non_pawns = our_pieces & ~this->pawns & from_mask;
        for (Square from_square : scan_reversed(non_pawns))
        {
            Bitboard moves = this->attacks_mask(from_square) & ~our_pieces & to_mask;
            for (Square to_square : scan_reversed(moves))
            {
                iter.push_back(Move(from_square, to_square));
            }
        }

        // Generate castling moves.
        if (from_mask & this->kings)
        {
            for (const Move &move : this->generate_castling_moves(from_mask, to_mask))
            {
                iter.push_back(move);
            }
        }

        // The remaining moves are all pawn moves.
        Bitboard pawns = this->pawns & this->occupied_co[this->turn] & from_mask;
        if (!pawns)
        {
            return iter;
        }

        // Generate pawn captures.
        Bitboard capturers = pawns;
        for (Square from_square : scan_reversed(capturers))
        {
            Bitboard targets = (BB_PAWN_ATTACKS[this->turn][from_square] &
                                this->occupied_co[!this->turn] & to_mask);

            for (Square to_square : scan_reversed(targets))
            {
                if (square_rank(to_square) == 0 || square_rank(to_square) == 7)
                {
                    iter.push_back(Move(from_square, to_square, QUEEN));
                    iter.push_back(Move(from_square, to_square, ROOK));
                    iter.push_back(Move(from_square, to_square, BISHOP));
                    iter.push_back(Move(from_square, to_square, KNIGHT));
                }
                else
                {
                    iter.push_back(Move(from_square, to_square));
                }
            }
        }

        // Prepare pawn advance generation.
        Bitboard single_moves, double_moves;
        if (this->turn == WHITE)
        {
            single_moves = pawns << 8 & ~this->occupied;
            double_moves = single_moves << 8 & ~this->occupied & (BB_RANK_3 | BB_RANK_4);
        }
        else
        {
            single_moves = pawns >> 8 & ~this->occupied;
            double_moves = single_moves >> 8 & ~this->occupied & (BB_RANK_6 | BB_RANK_5);
        }

        single_moves &= to_mask;
        double_moves &= to_mask;

        // Generate single pawn moves.
        for (Square to_square : scan_reversed(single_moves))
        {
            Square from_square = to_square + (this->turn == BLACK ? 8 : -8);

            if (square_rank(to_square) == 0 || square_rank(to_square) == 7)
            {
                iter.push_back(Move(from_square, to_square, QUEEN));
                iter.push_back(Move(from_square, to_square, ROOK));
                iter.push_back(Move(from_square, to_square, BISHOP));
                iter.push_back(Move(from_square, to_square, KNIGHT));
            }
            else
            {
                iter.push_back(Move(from_square, to_square));
            }
        }

        // Generate double pawn moves.
        for (Square to_square : scan_reversed(double_moves))
        {
            Square from_square = to_square + (this->turn == BLACK ? 16 : -16);
            iter.push_back(Move(from_square, to_square));
        }

        // Generate en passant captures.
        if (this->ep_square)
        {
            for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask))
            {
                iter.push_back(move);
            }
        }
        return iter;
    }

    std::vector<Move> Board::generate_pseudo_legal_ep(Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;
        if (!this->ep_square || !(BB_SQUARES[*this->ep_square] & to_mask))
        {
            return iter;
        }

        if (BB_SQUARES[*this->ep_square] & this->occupied)
        {
            return iter;
        }

        Bitboard capturers = (this->pawns & this->occupied_co[this->turn] & from_mask &
                              BB_PAWN_ATTACKS[!this->turn][*this->ep_square] &
                              BB_RANKS[this->turn ? 4 : 3]);

        for (Square capturer : scan_reversed(capturers))
        {
            iter.push_back(Move(capturer, *this->ep_square));
        }
        return iter;
    }

    std::vector<Move> Board::generate_pseudo_legal_captures(Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;
        for (const Move &move : this->generate_pseudo_legal_moves(from_mask, to_mask & this->occupied_co[!this->turn]))
        {
            iter.push_back(move);
        }
        for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask))
        {
            iter.push_back(move);
        }
        return iter;
    }

    Bitboard Board::checkers_mask() const
    {
        std::optional<Square> king = this->king(this->turn);
        return king == std::nullopt ? BB_EMPTY : this->attackers_mask(!this->turn, *king);
    }

    bool Board::is_check() const
    {
        /* Tests if the current side to move is in check. */
        return bool(this->checkers_mask());
    }

    bool Board::gives_check(const Move &move)
    {
        /*
        Probes if the given move would put the opponent in check. The move
        must be at least pseudo-legal.
        */
        this->push(move);
        bool is_check = this->is_check();
        this->pop();
        return is_check;
    }

    bool Board::is_into_check(const Move &move) const
    {
        std::optional<Square> king = this->king(this->turn);
        if (king == std::nullopt)
        {
            return false;
        }

        // If already in check, look if it is an evasion.
        Bitboard checkers = this->attackers_mask(!this->turn, *king);
        std::vector<Move> evasions = this->_generate_evasions(*king, checkers, BB_SQUARES[move.from_square], BB_SQUARES[move.to_square]);
        if (checkers && std::find(std::begin(evasions), std::end(evasions), move) == std::end(evasions))
        {
            return true;
        }

        return !this->_is_safe(*king, this->_slider_blockers(*king), move);
    }

    bool Board::was_into_check() const
    {
        std::optional<Square> king = this->king(!this->turn);
        return king != std::nullopt && this->is_attacked_by(this->turn, *king);
    }

    bool Board::is_pseudo_legal(Move move) const
    {
        // Null moves are not pseudo-legal.
        if (!move)
        {
            return false;
        }

        // Drops are not pseudo-legal.
        if (move.drop)
        {
            return false;
        }

        // Source square must not be vacant.
        std::optional<PieceType> piece = this->piece_type_at(move.from_square);
        if (!piece)
        {
            return false;
        }

        // Get square masks.
        Bitboard from_mask = BB_SQUARES[move.from_square];
        Bitboard to_mask = BB_SQUARES[move.to_square];

        // Check turn.
        if (!(this->occupied_co[this->turn] & from_mask))
        {
            return false;
        }

        // Only pawns can promote and only on the backrank.
        if (move.promotion)
        {
            if (*piece != PAWN)
            {
                return false;
            }

            if (this->turn == WHITE && square_rank(move.to_square) != 7)
            {
                return false;
            }
            else if (this->turn == BLACK && square_rank(move.to_square) != 0)
            {
                return false;
            }
        }

        // Handle castling.
        if (*piece == KING)
        {
            move = this->_from_chess960(this->chess960, move.from_square, move.to_square);
            std::vector<Move> castling_moves = this->generate_castling_moves();
            if (std::find(std::begin(castling_moves), std::end(castling_moves), move) != std::end(castling_moves))
            {
                return true;
            }
        }

        // Destination square can not be occupied.
        if (this->occupied_co[this->turn] & to_mask)
        {
            return false;
        }

        // Handle pawn moves.
        if (*piece == PAWN)
        {
            std::vector<Move> pseudo_legal_moves = this->generate_pseudo_legal_moves(from_mask, to_mask);
            return std::find(std::begin(pseudo_legal_moves), std::end(pseudo_legal_moves), move) != std::end(pseudo_legal_moves);
        }

        // Handle all other pieces.
        return bool(this->attacks_mask(move.from_square) & to_mask);
    }

    bool Board::is_legal(const Move &move) const
    {
        return this->is_pseudo_legal(move) && !this->is_into_check(move);
    }

    bool Board::is_game_over(bool claim_draw)
    {
        return this->generate_legal_moves().empty();
    }

    std::string Board::result(bool claim_draw)
    {
        if (this->is_checkmate())
        {
            return !this->turn?"1-0":"0-1";
        }
        if (this->is_insufficient_material())
        {
            return "1/2-1/2";
        }
        if (this->generate_legal_moves().empty())
        {
            return "1/2-1/2";
        }

        // Automatic draws.
        if (this->is_seventyfive_moves())
        {
            return "1/2-1/2";
        }
        if (this->is_fivefold_repetition())
        {
            return "1/2-1/2";
        }

        // Claimable draws.
        if (claim_draw)
        {
            if (this->can_claim_fifty_moves())
            {
                return "1/2-1/2";
            }
            if (this->can_claim_threefold_repetition())
            {
                return "1/2-1/2";
            }
        }
        return "*";
    }

    bool Board::is_checkmate() const
    {
        /* Checks if the current position is a checkmate. */
        if (!this->is_check())
        {
            return false;
        }

        return this->generate_legal_moves().empty();
    }

    bool Board::is_stalemate() const
    {
        /* Checks if the current position is a stalemate. */
        if (this->is_check())
        {
            return false;
        }

        return this->generate_legal_moves().empty();
    }

    bool Board::is_insufficient_material() const
    {
        /*
        Checks if neither side has sufficient winning material
        (:func:`~Board::has_insufficient_material()`).
        */
        return this->has_insufficient_material(WHITE) && this->has_insufficient_material(BLACK);
    }

    bool Board::has_insufficient_material(Color color) const
    {
        /*
        Checks if *color* has insufficient winning material.

        This is guaranteed to return ``false`` if *color* can still win the
        game.

        The converse does not necessarily hold:
        The implementation only looks at the material, including the colors
        of bishops, but not considering piece positions. So fortress
        positions or positions with forced lines may return ``false``, even
        though there is no possible winning line.
        */
        if (this->occupied_co[color] & (this->pawns | this->rooks | this->queens))
        {
            return false;
        }

        // Knights are only insufficient material if:
        // (1) We do not have any other pieces, including more than one knight.
        // (2) The opponent does not have pawns, knights, bishops or rooks.
        //     These would allow selfmate.
        if (this->occupied_co[color] & this->knights)
        {
            return (popcount(this->occupied_co[color]) <= 2 &&
                    !(this->occupied_co[!color] & ~this->kings & ~this->queens));
        }

        // Bishops are only insufficient material if:
        // (1) We do not have any other pieces, including bishops of the
        //     opposite color.
        // (2) The opponent does not have bishops of the opposite color,
        //     pawns or knights. These would allow selfmate.
        if (this->occupied_co[color] & this->bishops)
        {
            bool same_color = (!(this->bishops & BB_DARK_SQUARES)) || (!(this->bishops & BB_LIGHT_SQUARES));
            return same_color && !this->pawns && !this->knights;
        }

        return true;
    }

    bool Board::is_seventyfive_moves() const
    {
        /*
        Since the 1st of July 2014, a game is automatically drawn (without
        a claim by one of the players) if the half-move clock since a capture
        or pawn move is equal to or greater than 150. Other means to end a game
        take precedence.
        */
        return this->_is_halfmoves(150);
    }

    bool Board::is_fivefold_repetition()
    {
        /*
        Since the 1st of July 2014 a game is automatically drawn (without
        a claim by one of the players) if a position occurs for the fifth time.
        Originally this had to occur on consecutive alternating moves, but
        this has since been revised.
        */
        return this->is_repetition(5);
    }

    bool Board::can_claim_draw()
    {
        /*
        Checks if the player to move can claim a draw by the fifty-move rule or
        by threefold repetition.

        Note that checking the latter can be slow.
        */
        return this->can_claim_fifty_moves() || this->can_claim_threefold_repetition();
    }

    bool Board::is_fifty_moves() const
    {
        return this->_is_halfmoves(100);
    }

    bool Board::can_claim_fifty_moves()
    {
        /*
        Checks if the player to move can claim a draw by the fifty-move rule.

        Draw by the fifty-move rule can be claimed once the clock of halfmoves
        since the last capture or pawn move becomes equal or greater to 100,
        or if there is a legal move that achieves this. Other means of ending
        the game take precedence.
        */
        if (this->is_fifty_moves())
        {
            return true;
        }

        if (this->halfmove_clock >= 99)
        {
            for (const Move &move : this->generate_legal_moves())
            {
                if (!this->is_zeroing(move))
                {
                    this->push(move);
                    bool is_fifty_moves = this->is_fifty_moves();
                    this->pop();
                    if (this->is_fifty_moves())
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool Board::can_claim_threefold_repetition()
    {
        /*
        Checks if the player to move can claim a draw by threefold repetition.

        Draw by threefold repetition can be claimed if the position on the
        board occured for the third time or if such a repetition is reached
        with one of the possible legal moves.

        Note that checking this can be slow: In the worst case
        scenario, every legal move has to be tested and the entire game has to
        be replayed because there is no incremental transposition table.
        */
        auto transposition_key = this->_transposition_key();

        struct transposition_hash
        {
            size_t operator()(const std::tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, Square> &key) const
            {
                return std::hash<Bitboard>()(std::get<0>(key)) ^ std::hash<Bitboard>()(std::get<1>(key)) ^ std::hash<Bitboard>()(std::get<2>(key)) ^ std::hash<Bitboard>()(std::get<3>(key)) ^ std::hash<Bitboard>()(std::get<4>(key)) ^ std::hash<Bitboard>()(std::get<5>(key)) ^ std::hash<Bitboard>()(std::get<6>(key)) ^ std::hash<Bitboard>()(std::get<7>(key)) ^ std::hash<Color>()(std::get<8>(key)) ^ std::hash<Bitboard>()(std::get<9>(key)) ^ std::hash<Square>()(std::get<10>(key));
            }
        };

        std::unordered_map<std::tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, Square>, int, transposition_hash> transpositions;
        ++transpositions[transposition_key];

        // Count positions.
        std::stack<Move> switchyard;
        while (!this->move_stack.empty())
        {
            Move move = this->pop();
            switchyard.push(move);

            if (this->is_irreversible(move))
            {
                break;
            }

            ++transpositions[this->_transposition_key()];
        }

        while (!switchyard.empty())
        {
            this->push(switchyard.top());
            switchyard.pop();
        }

        // Threefold repetition occured.
        if (transpositions.at(transposition_key) >= 3)
        {
            return true;
        }

        // The next legal move is a threefold repetition.
        for (const Move &move : this->generate_legal_moves())
        {
            this->push(move);
            bool flag = transpositions.find(this->_transposition_key()) != std::end(transpositions) && transpositions.at(this->_transposition_key()) >= 2;
            this->pop();
            if (flag)
            {
                return true;
            }
        }

        return false;
    }

    bool Board::is_repetition(int count)
    {
        /*
        Checks if the current position has repeated 3 (or a given number of)
        times.

        Unlike :func:`~Board::can_claim_threefold_repetition()`,
        this does not consider a repetition that can be played on the next
        move.

        Note that checking this can be slow: In the worst case, the entire
        game has to be replayed because there is no incremental transposition
        table.
        */
        // Fast check, based on occupancy only.
        int maybe_repetitions = 1;
        for (auto it = rbegin(this->_stack); it != rend(this->_stack); ++it)
        {
            _BoardState state = *it;
            if (state.occupied == this->occupied)
            {
                ++maybe_repetitions;
                if (maybe_repetitions >= count)
                {
                    break;
                }
            }
        }
        if (maybe_repetitions < count)
        {
            return false;
        }

        // Check full replay.
        auto transposition_key = this->_transposition_key();
        std::stack<Move> switchyard;

        bool flag = false;
        while (true)
        {
            if (count <= 1)
            {
                flag = true;
                break;
            }

            if (this->move_stack.size() < count - 1)
            {
                break;
            }

            Move move = this->pop();
            switchyard.push(move);

            if (this->is_irreversible(move))
            {
                break;
            }

            if (this->_transposition_key() == transposition_key)
            {
                --count;
            }
        }
        while (!switchyard.empty())
        {
            this->push(switchyard.top());
            switchyard.pop();
        }

        return flag;
    }

    void Board::push(Move move)
    {
        /*
        Updates the position with the given *move* and puts it onto the
        move stack.

        >>> #include "chess.cpp"
        >>> #include <iostream>
        >>>
        >>> Board board;
        >>>
        >>> Move Nf3 = Move::from_uci("g1f3");
        >>> board.push(Nf3);  // Make the move

        >>> std::cout << board.pop();  // Unmake the last move
        Move::from_uci("g1f3")

        Null moves just increment the move counters, switch turns and forfeit
        en passant capturing.

        .. warning::
            Moves are not checked for legality. It is the caller's
            responsibility to ensure that the move is at least pseudo-legal or
            a null move.
        */
        // Push move and remember board state.
        move = this->_to_chess960(move);
        _BoardState board_state = this->_board_state();
        this->castling_rights = this->clean_castling_rights(); // Before pushing stack
        this->move_stack.push_back(this->_from_chess960(this->chess960, move.from_square, move.to_square, move.promotion, move.drop));
        this->_stack.push_back(board_state);

        // Reset en passant square.
        std::optional<Square> ep_square = this->ep_square;
        this->ep_square = std::nullopt;

        // Increment move counters.
        ++this->halfmove_clock;
        if (this->turn == BLACK)
        {
            ++this->fullmove_number;
        }

        // On a null move, simply swap turns and reset the en passant square.
        if (!move)
        {
            this->turn = !this->turn;
            return;
        }

        // Drops.
        if (move.drop)
        {
            this->_set_piece_at(move.to_square, *move.drop, this->turn);
            this->turn = !this->turn;
            return;
        }

        // Zero the half-move clock.
        if (this->is_zeroing(move))
        {
            this->halfmove_clock = 0;
        }

        Bitboard from_bb = BB_SQUARES[move.from_square];
        Bitboard to_bb = BB_SQUARES[move.to_square];

        bool promoted = bool(this->promoted & from_bb);
        std::optional<PieceType> piece_type = this->_remove_piece_at(move.from_square);
        if (piece_type == std::nullopt)
        {
            throw std::runtime_error("push() expects move to be pseudo-legal, but got " + std::string(move) + " in " + this->board_fen());
        }
        Square capture_square = move.to_square;
        std::optional<PieceType> captured_piece_type = this->piece_type_at(capture_square);

        // Update castling rights.
        this->castling_rights &= ~to_bb & ~from_bb;
        if (*piece_type == KING && !promoted)
        {
            if (this->turn == WHITE)
            {
                this->castling_rights &= ~BB_RANK_1;
            }
            else
            {
                this->castling_rights &= ~BB_RANK_8;
            }
        }
        else if (captured_piece_type && *captured_piece_type == KING && !(this->promoted & to_bb))
        {
            if (this->turn == WHITE && square_rank(move.to_square) == 7)
            {
                this->castling_rights &= ~BB_RANK_8;
            }
            else if (this->turn == BLACK && square_rank(move.to_square) == 0)
            {
                this->castling_rights &= ~BB_RANK_1;
            }
        }

        // Handle special pawn moves.
        if (*piece_type == PAWN)
        {
            int diff = move.to_square - move.from_square;

            if (diff == 16 && square_rank(move.from_square) == 1)
            {
                this->ep_square = move.from_square + 8;
            }
            else if (diff == -16 && square_rank(move.from_square) == 6)
            {
                this->ep_square = move.from_square - 8;
            }
            else if (move.to_square == *ep_square && (abs(diff) == 7 || abs(diff) == 9) && !captured_piece_type)
            {
                // Remove pawns captured en passant.
                int down = this->turn == WHITE ? -8 : 8;
                capture_square = *ep_square + down;
                captured_piece_type = this->_remove_piece_at(capture_square);
            }
        }

        // Promotion.
        if (move.promotion)
        {
            promoted = true;
            piece_type = move.promotion;
        }

        // Castling.
        bool castling = *piece_type == KING && this->occupied_co[this->turn] & to_bb;
        if (castling)
        {
            bool a_side = square_file(move.to_square) < square_file(move.from_square);

            this->_remove_piece_at(move.from_square);
            this->_remove_piece_at(move.to_square);

            if (a_side)
            {
                this->_set_piece_at(this->turn == WHITE ? C1 : C8, KING, this->turn);
                this->_set_piece_at(this->turn == WHITE ? D1 : D8, ROOK, this->turn);
            }
            else
            {
                this->_set_piece_at(this->turn == WHITE ? G1 : G8, KING, this->turn);
                this->_set_piece_at(this->turn == WHITE ? F1 : F8, ROOK, this->turn);
            }
        }

        // Put the piece on the target square.
        if (!castling)
        {
            bool was_promoted = bool(this->promoted & to_bb);
            this->_set_piece_at(move.to_square, *piece_type, this->turn, promoted);

            /*if (captured_piece_type)
            {
                this->_push_capture(move, capture_square, *captured_piece_type, was_promoted);
            }*/
        }

        // Swap turn.
        this->turn = !this->turn;
    }

    Move Board::pop()
    {
        /*
        Restores the previous position and returns the last move from the stack.

        :throws: :exc:`std::out_of_range` if the move stack is empty.
        */
        if (this->move_stack.empty())
        {
            throw std::out_of_range("");
        }
        Move move = this->move_stack.back();
        this->move_stack.pop_back();
        this->_stack.back().restore(*this);
        this->_stack.pop_back();
        return move;
    }

    Move Board::peek() const
    {
        /*
        Gets the last move from the move stack.

        :throws: :exc:`std::out_of_range` if the move stack is empty.
        */
        if (this->move_stack.empty())
        {
            throw std::out_of_range("");
        }
        return this->move_stack.back();
    }

    Move Board::find_move(Square from_square, Square to_square, std::optional<PieceType> promotion)
    {
        /*
        Finds a matching legal move for an origin square, a target square, and
        an optional promotion piece type.

        For pawn moves to the backrank, the promotion piece type defaults to
        :data:`QUEEN`, unless otherwise specified.

        Castling moves are normalized to king moves by two steps, except in
        Chess960.

        :throws: :exc:`std::invalid_argument` if no matching legal move is found.
        */
        if (promotion == std::nullopt && this->pawns & BB_SQUARES[from_square] && BB_SQUARES[to_square] & BB_BACKRANKS)
        {
            promotion = QUEEN;
        }

        Move move = this->_from_chess960(this->chess960, from_square, to_square, promotion);
        if (!this->is_legal(move))
        {
            throw std::invalid_argument("no matching legal move for " + move.uci() + " (" + SQUARE_NAMES[from_square] + " -> " + SQUARE_NAMES[to_square] + " in " + this->fen());
        }

        return move;
    }

    std::string Board::castling_shredder_fen() const
    {
        Bitboard castling_rights = this->clean_castling_rights();
        if (!castling_rights)
        {
            return "-";
        }

        std::vector<char> builder;

        for (Square square : scan_reversed(castling_rights & BB_RANK_1))
        {
            builder.push_back(std::toupper(FILE_NAMES[square_file(square)]));
        }

        for (Square square : scan_reversed(castling_rights & BB_RANK_8))
        {
            builder.push_back(FILE_NAMES[square_file(square)]);
        }

        return std::string(std::begin(builder), std::end(builder));
    }

    std::string Board::castling_xfen() const
    {
        std::vector<char> builder;

        for (Color color : COLORS)
        {
            std::optional<Square> king = this->king(color);
            if (king == std::nullopt)
            {
                continue;
            }

            int king_file = square_file(*king);
            Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;

            for (Square rook_square : scan_reversed(this->clean_castling_rights() & backrank))
            {
                int rook_file = square_file(rook_square);
                bool a_side = rook_file < king_file;

                Bitboard other_rooks = this->occupied_co[color] & this->rooks & backrank & ~BB_SQUARES[rook_square];

                char ch = a_side ? 'q' : 'k';
                for (Square other : scan_reversed(other_rooks))
                {
                    if ((square_file(other) < rook_file) == a_side)
                    {
                        ch = FILE_NAMES[rook_file];
                        break;
                    }
                }

                builder.push_back(color == WHITE ? std::toupper(ch) : ch);
            }
        }

        if (!builder.empty())
        {
            return std::string(std::begin(builder), std::end(builder));
        }
        else
        {
            return "-";
        }
    }

    bool Board::has_pseudo_legal_en_passant() const
    {
        /* Checks if there is a pseudo-legal en passant capture. */
        return this->ep_square != std::nullopt && !this->generate_pseudo_legal_ep().empty();
    }

    bool Board::has_legal_en_passant() const
    {
        /* Checks if there is a legal en passant capture. */
        return this->ep_square != std::nullopt && !this->generate_legal_ep().empty();
    }

    std::string Board::fen(bool shredder, _EnPassantSpec en_passant, std::optional<bool> promoted)
    {
        /*
        Gets a FEN representation of the position.

        A FEN string (e.g.,
        ``rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1``) consists
        of the board part :func:`~Board::board_fen()`, the
        :data:`~Board::turn`, the castling part
        (:data:`~Board::castling_rights`),
        the en passant square (:data:`~Board::ep_square`),
        the :data:`~Board::halfmove_clock`
        and the :data:`~Board::fullmove_number`.

        :param shredder: Use :func:`~Board::castling_shredder_fen()`
            and encode castling rights by the file of the rook
            (like ``HAha``) instead of the default
            :func:`~Board::castling_xfen()` (like ``KQkq``).
        :param en_passant: By default, only fully legal en passant squares
            are included (:func:`~Board::has_legal_en_passant()`).
            Pass ``fen`` to strictly follow the FEN specification
            (always include the en passant square after a two-step pawn move)
            or ``xfen`` to follow the X-FEN specification
            (:func:`~Board::has_pseudo_legal_en_passant()`).
        :param promoted: Mark promoted pieces like ``Q~``. By default, this is
            only enabled in chess variants where this is relevant.
        */
        return this->epd(shredder, en_passant, promoted) + " " + std::to_string(this->halfmove_clock) + " " + std::to_string(this->fullmove_number);
    }

    std::string Board::shredder_fen(_EnPassantSpec en_passant, std::optional<bool> promoted)
    {
        return this->epd(true, en_passant, promoted) + " " + std::to_string(this->halfmove_clock) + " " + std::to_string(this->fullmove_number);
    }

    void Board::set_fen(const std::string &fen)
    {
        /*
        Parses a FEN and sets the position from it.

        :throws: :exc:`std::invalid_argument` if syntactically invalid. Use
            :func:`~Board::is_valid()` to detect invalid positions.
        */
        std::istringstream iss(fen);
        std::deque<std::string> parts = {std::istream_iterator<std::string>(iss), {}};

        // Board part.
        std::string board_part;
        if (!parts.empty())
        {
            board_part = parts.front();
            parts.pop_front();
        }
        else
        {
            throw std::invalid_argument("empty fen");
        }

        // Turn.
        std::string turn_part;
        Color turn;
        if (!parts.empty())
        {
            turn_part = parts.front();
            parts.pop_front();

            if (turn_part == "w")
            {
                turn = WHITE;
            }
            else if (turn_part == "b")
            {
                turn = BLACK;
            }
            else
            {
                throw std::invalid_argument("expected 'w' or 'b' for turn part of fen: \"" + fen + "\"");
            }
        }
        else
        {
            turn = WHITE;
        }

        // Validate castling part.
        std::string castling_part;
        if (!parts.empty())
        {
            castling_part = parts.front();
            parts.pop_front();

            if (!regex_match(castling_part, FEN_CASTLING_REGEX))
            {
                throw std::invalid_argument("invalid castling part in fen: \"" + fen + "\"");
            }
        }
        else
        {
            castling_part = "-";
        }

        // En passant square.
        std::optional<std::string> ep_part;
        std::optional<Square> ep_square;
        if (!parts.empty())
        {
            ep_part = parts.front();
            parts.pop_front();

            if (ep_part != "-")
            {
                auto it = std::find(std::begin(SQUARE_NAMES), std::end(SQUARE_NAMES), ep_part);
                if (it == std::end(SQUARE_NAMES))
                {
                    throw std::invalid_argument("invalid en passant part in fen: \"" + fen + "\"");
                }
                ep_square = std::distance(SQUARE_NAMES, it);
            }
            else
            {
                ep_square = std::nullopt;
            }
        }
        else
        {
            ep_square = std::nullopt;
        }

        // Check that the half-move part is valid.
        std::string halfmove_part;
        int halfmove_clock;
        if (!parts.empty())
        {
            halfmove_part = parts.front();
            parts.pop_front();

            try
            {
                halfmove_clock = stoi(halfmove_part);
            }
            catch (std::invalid_argument)
            {
                throw std::invalid_argument("invalid half-move clock in fen: \"" + fen + "\"");
            }

            if (halfmove_clock < 0)
            {
                throw std::invalid_argument("half-move clock cannot be negative: \"" + fen + "\"");
            }
        }
        else
        {
            halfmove_clock = 0;
        }

        // Check that the full-move number part is valid.
        // 0 is allowed for compatibility, but later replaced with 1.
        std::string fullmove_part;
        int fullmove_number;
        if (!parts.empty())
        {
            fullmove_part = parts.front();
            parts.pop_front();

            try
            {
                fullmove_number = stoi(fullmove_part);
            }
            catch (std::invalid_argument)
            {
                throw std::invalid_argument("invalid fullmove number in fen: \"" + fen + "\"");
            }

            if (fullmove_number < 0)
            {
                throw std::invalid_argument("fullmove number cannot be negative: \"" + fen + "\"");
            }

            fullmove_number = std::max(fullmove_number, 1);
        }
        else
        {
            fullmove_number = 1;
        }

        // All parts should be consumed now.
        if (!parts.empty())
        {
            throw std::invalid_argument("fen string has more parts than expected: \"" + fen + "\"");
        }

        // Validate the board part and set it.
        this->_set_board_fen(board_part);

        // Apply.
        this->turn = turn;
        this->_set_castling_fen(castling_part);
        this->ep_square = ep_square;
        this->halfmove_clock = halfmove_clock;
        this->fullmove_number = fullmove_number;
        this->clear_stack();
    }

    void Board::set_castling_fen(const std::string &castling_fen)
    {
        /*
        Sets castling rights from a string in FEN notation like ``Qqk``.

        :throws: :exc:`std::invalid_argument` if the castling FEN is syntactically
            invalid.
        */
        this->_set_castling_fen(castling_fen);
        this->clear_stack();
    }

    void Board::set_board_fen(const std::string &fen)
    {
        BaseBoard::set_board_fen(fen);
        this->clear_stack();
    }

    std::string Board::epd(bool shredder, const _EnPassantSpec &en_passant, std::optional<bool> promoted, const std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> &operations)
    {
        /*
        Gets an EPD representation of the current position.

        See :func:`~Board::fen()` for FEN formatting options (*shredder*,
        *ep_square* and *promoted*).

        Supported operands for EPD operations
        are strings, integers, finite floats, legal moves and ``std::nullopt``.
        Additionally, the operation ``pv`` accepts a legal variation as
        a list of moves. The operations ``am`` and ``bm`` accept a list of
        legal moves in the current position.

        The name of the field cannot be a lone dash and cannot contain spaces,
        newlines, carriage returns or tabs.

        *hmvc* and *fmvn* are not included by default. You can use:

        >>> #include "chess.cpp"
        >>> #include <iostream>
        >>>
        >>> Board board;
        >>> std::cout << board.epd(false, "legal", std::nullopt, {{"hmvc", board.halfmove_clock}, {"fmvn", board.fullmove_number}});
        rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - hmvc 0; fmvn 1;
        */
        std::optional<Square> ep_square;
        if (en_passant == "fen")
        {
            ep_square = this->ep_square;
        }
        else if (en_passant == "xfen")
        {
            ep_square = this->has_pseudo_legal_en_passant() ? this->ep_square : std::nullopt;
        }
        else
        {
            ep_square = this->has_legal_en_passant() ? this->ep_square : std::nullopt;
        }

        std::vector<std::string> epd = {this->board_fen(promoted),
                                        this->turn == WHITE ? "w" : "b",
                                        shredder ? this->castling_shredder_fen() : this->castling_xfen(),
                                        ep_square != std::nullopt ? SQUARE_NAMES[*ep_square] : "-"};

        if (!operations.empty())
        {
            epd.push_back(this->_epd_operations(operations));
        }

        std::string s;
        for (const std::string &s2 : epd)
        {
            s += s2;
            s += " ";
        }
        s.resize(s.size() - 1);
        return s;
    }

    std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> Board::set_epd(const std::string &epd)
    {
        /*
        Parses the given EPD string and uses it to set the position.

        If present, ``hmvc`` and ``fmvn`` are used to set the half-move
        clock and the full-move number. Otherwise, ``0`` and ``1`` are used.

        Returns a dictionary of parsed operations. Values can be strings,
        integers, floats, move objects, or lists of moves.

        :throws: :exc:`std::invalid_argument` if the EPD string is invalid.
        */
        auto it = begin(epd);
        auto it2 = rbegin(epd);
        while (isspace(*it))
        {
            ++it;
        }
        while (isspace(*it2) || *it2 == ';')
        {
            ++it2;
        }
        std::vector<std::string> parts;
        std::string s = std::string(it, it2.base());
        std::istringstream iss(s);
        std::string s2;
        for (int i = 0; getline(iss, s2, ' ') && i < 4; ++i)
        {
            parts.push_back(s2);
        }
        int i, splits;
        for (i = 0, splits = 0; i < s.length() && splits < 4; ++i)
        {
            if (isspace(s[i]))
            {
                ++splits;
                ++i;
                while (i < s.length() && isspace(s[i]))
                {
                    ++i;
                }
            }
        }
        parts.push_back(s.substr(i - 1));

        // Parse ops.
        if (parts.size() > 4)
        {
            std::string back = parts.back();
            parts.pop_back();
            std::string joined;
            for (std::string s : parts)
            {
                joined += s;
                joined += " ";
            }
            joined.resize(joined.size() - 1);
            auto operations = this->_parse_epd_ops(back, [&]() -> Board
                                                   { return Board(joined + " 0 1"); });
            if (operations.find("hmvc") != std::end(operations))
            {
                if (std::holds_alternative<std::string>(operations.at("hmvc")))
                {
                    parts.push_back(std::get<std::string>(operations.at("hmvc")));
                }
                else if (std::holds_alternative<int>(operations.at("hmvc")))
                {
                    parts.push_back(std::to_string(std::get<int>(operations.at("hmvc"))));
                }
                else
                {
                    parts.push_back(std::to_string(int(std::get<float>(operations.at("hmvc")))));
                }
            }
            else
            {
                parts.push_back("0");
            }
            if (operations.find("fmvn") != std::end(operations))
            {
                if (std::holds_alternative<std::string>(operations.at("fmvn")))
                {
                    parts.push_back(std::get<std::string>(operations.at("fmvn")));
                }
                else if (std::holds_alternative<int>(operations.at("fmvn")))
                {
                    parts.push_back(std::to_string(std::get<int>(operations.at("fmvn"))));
                }
                else
                {
                    parts.push_back(std::to_string(int(std::get<float>(operations.at("fmvn")))));
                }
            }
            else
            {
                parts.push_back("1");
            }
            joined = "";
            for (std::string s : parts)
            {
                joined += s;
                joined += " ";
            }
            joined.resize(joined.size() - 1);
            this->set_fen(joined);
            return operations;
        }
        else
        {
            this->set_fen(epd);
            return {};
        }
    }

    std::string Board::san(const Move &move)
    {
        /*
        Gets the standard algebraic notation of the given move in the context
        of the current position.
        */
        return this->_algebraic(move);
    }

    std::string Board::lan(const Move &move)
    {
        /*
        Gets the long algebraic notation of the given move in the context of
        the current position.
        */
        return this->_algebraic(move, true);
    }

    std::string Board::san_and_push(const Move &move)
    {
        return this->_algebraic_and_push(move);
    }

    std::string Board::variation_san(const std::vector<Move> &variation) const
    {
        /*
        Given a sequence of moves, returns a string representing the sequence
        in standard algebraic notation (e.g., ``1. e4 e5 2. Nf3 Nc6`` or
        ``37...Bg6 38. fxg6``).

        The board will not be modified as a result of calling this.

        :throws: :exc:`std::invalid_argument` if any moves in the sequence are illegal.
        */
        Board board = this->copy(false);
        std::vector<std::string> san;

        for (const Move &move : variation)
        {
            if (!board.is_legal(move))
            {
                throw std::invalid_argument("illegal move " + std::string(move) + " in position " + board.fen());
            }

            if (board.turn == WHITE)
            {
                san.push_back(std::to_string(board.fullmove_number) + ". " + board.san_and_push(move));
            }
            else if (san.empty())
            {
                san.push_back(std::to_string(board.fullmove_number) + "..." + board.san_and_push(move));
            }
            else
            {
                san.push_back(board.san_and_push(move));
            }
        }

        std::string joined;
        for (const std::string &s : san)
        {
            joined += s;
        }
        return joined;
    }

    Move Board::parse_san(const std::string &san)
    {
        /*
        Uses the current position as the context to parse a move in standard
        algebraic notation and returns the corresponding move object.

        Ambiguous moves are rejected. Overspecified moves (including long
        algebraic notation) are accepted.

        The returned move is guaranteed to be either legal or a null move.

        :throws: :exc:`std::invalid_argument` if the SAN is invalid, illegal or ambiguous.
        */
        // Castling.
        try
        {
            if (san == "O-O" || san == "O-O+" || san == "O-O#" || san == "0-0" || san == "0-0+" || san == "0-0#")
            {
                for (const Move &move : this->generate_castling_moves())
                {
                    if (this->is_kingside_castling(move))
                    {
                        return move;
                    }
                }
                throw std::out_of_range("");
            }
            else if (san == "O-O-O" || san == "O-O-O+" || san == "O-O-O#" || san == "0-0-0" || san == "0-0-0+" || san == "0-0-0#")
            {
                for (const Move &move : this->generate_castling_moves())
                {
                    if (this->is_queenside_castling(move))
                    {
                        return move;
                    }
                }
                throw std::out_of_range("");
            }
        }
        catch (std::out_of_range)
        {
            throw std::invalid_argument("illegal san: \"" + san + "\" in" + this->fen());
        }

        // Match normal moves.
        std::smatch match;
        std::regex_search(san, match, SAN_REGEX);
        if (match.empty())
        {
            // Null moves.
            if (san == "--" || san == "Z0" || san == "0000" || san == "@@@@")
            {
                return Move::null();
            }
            else if (san.find(',') != std::string::npos)
            {
                throw std::invalid_argument("unsupported multi-leg move: \"" + san + "\"");
            }
            else
            {
                throw std::invalid_argument("invalid san: \"" + san + "\"");
            }
        }

        // Get target square. Mask our own pieces to exclude castling moves.
        auto it = std::find(std::begin(SQUARE_NAMES), std::end(SQUARE_NAMES), match[4].str());
        if (it == std::end(SQUARE_NAMES))
        {
            throw std::invalid_argument("");
        }
        Square to_square = std::distance(SQUARE_NAMES, it);
        Bitboard to_mask = BB_SQUARES[to_square] & ~this->occupied_co[this->turn];

        // Get the promotion piece type.
        std::string p = match[5].str();
        std::optional<PieceType> promotion;
        if (!p.empty())
        {
            auto it = std::find(std::begin(PIECE_SYMBOLS), std::end(PIECE_SYMBOLS), std::tolower(p.front()));
            if (it == std::end(PIECE_SYMBOLS))
            {
                throw std::invalid_argument("");
            }
            promotion = std::distance(PIECE_SYMBOLS, it);
        }
        else
        {
            promotion = std::nullopt;
        }

        // Filter by original square.
        int from_file;
        Bitboard from_mask = BB_ALL;
        if (!match[2].str().empty())
        {
            auto it = std::find(std::begin(FILE_NAMES), std::end(FILE_NAMES), match[2].str().front());
            if (it == std::end(FILE_NAMES))
            {
                throw std::invalid_argument("");
            }
            from_file = std::distance(FILE_NAMES, it);
            from_mask &= BB_FILES[from_file];
        }
        int from_rank;
        if (!match[3].str().empty())
        {
            from_rank = stoi(match[3].str()) - 1;
            from_mask &= BB_RANKS[from_rank];
        }

        // Filter by piece type.
        if (!match[1].str().empty())
        {
            auto it = std::find(std::begin(PIECE_SYMBOLS), std::end(PIECE_SYMBOLS), std::tolower(match[1].str().front()));
            if (it == std::end(PIECE_SYMBOLS) || match[1].str().length() > 1)
            {
                throw std::invalid_argument("");
            }
            PieceType piece_type = std::distance(PIECE_SYMBOLS, it);
            from_mask &= this->pieces_mask(piece_type, this->turn);
        }
        else if (!match[2].str().empty() && !match[3].str().empty())
        {
            // Allow fully specified moves, even if they are not pawn moves,
            // including castling moves.
            Move move = this->find_move(square(from_file, from_rank), to_square, promotion);
            if (move.promotion == promotion)
            {
                return move;
            }
            else
            {
                throw std::invalid_argument("missing promotion piece type: \"" + san + "\" in " + this->fen());
            }
        }
        else
        {
            from_mask &= this->pawns;
        }

        // Match legal moves.
        std::optional<Move> matched_move = std::nullopt;
        for (const Move &move : this->generate_legal_moves(from_mask, to_mask))
        {
            if (move.promotion != promotion)
            {
                continue;
            }

            if (matched_move)
            {
                throw std::invalid_argument("ambiguous san: \"" + san + "\" in " + this->fen());
            }

            matched_move = move;
        }

        if (!matched_move)
        {
            throw std::invalid_argument("illegal san: \"" + san + "\" in " + this->fen());
        }

        return *matched_move;
    }

    Move Board::push_san(const std::string &san)
    {
        /*
        Parses a move in standard algebraic notation, makes the move and puts
        it onto the move stack.

        Returns the move.

        :throws: :exc:`std::invalid_argument` if neither legal nor a null move.
        */
        Move move = this->parse_san(san);
        this->push(move);
        return move;
    }

    std::string Board::uci(Move move, std::optional<bool> chess960) const
    {
        /*
        Gets the UCI notation of the move.

        *chess960* defaults to the mode of the board. Pass ``true`` to force
        Chess960 mode.
        */
        if (chess960 == std::nullopt)
        {
            chess960 = this->chess960;
        }

        move = this->_to_chess960(move);
        move = this->_from_chess960(*chess960, move.from_square, move.to_square, move.promotion, move.drop);
        return move.uci();
    }

    Move Board::parse_uci(const std::string &uci)
    {
        /*
        Parses the given move in UCI notation.

        Supports both Chess960 and standard UCI notation.

        The returned move is guaranteed to be either legal or a null move.

        :throws: :exc:`std::invalid_argument` if the move is invalid or illegal in the
            current position (but not a null move).
        */
        Move move = Move::from_uci(uci);

        if (!move)
        {
            return move;
        }

        move = this->_to_chess960(move);
        move = this->_from_chess960(this->chess960, move.from_square, move.to_square, move.promotion, move.drop);

        if (!this->is_legal(move))
        {
            throw std::invalid_argument("illegal uci: \"" + uci + "\" in " + this->fen());
        }

        return move;
    }

    Move Board::push_uci(const std::string &uci)
    {
        /*
        Parses a move in UCI notation and puts it on the move stack.

        Returns the move.

        :throws: :exc:`std::invalid_argument` if the move is invalid or illegal in the
            current position (but not a null move).
        */
        Move move = this->parse_uci(uci);
        this->push(move);
        return move;
    }

    std::string Board::xboard(const Move &move, std::optional<bool> chess960) const
    {
        if (chess960 == std::nullopt)
        {
            chess960 = this->chess960;
        }

        if (!*chess960 || !this->is_castling(move))
        {
            return move.xboard();
        }
        else if (this->is_kingside_castling(move))
        {
            return "O-O";
        }
        else
        {
            return "O-O-O";
        }
    }

    Move Board::parse_xboard(const std::string &xboard)
    {
        return this->parse_san(xboard);
    }

    Move Board::push_xboard(const std::string &san)
    {
        /*
        Parses a move in standard algebraic notation, makes the move and puts
        it onto the move stack.

        Returns the move.

        :throws: :exc:`std::invalid_argument` if neither legal nor a null move.
        */
        Move move = this->parse_san(san);
        this->push(move);
        return move;
    }

    bool Board::is_en_passant(const Move &move) const
    {
        /* Checks if the given pseudo-legal move is an en passant capture. */
        return (this->ep_square == move.to_square &&
                bool(this->pawns & BB_SQUARES[move.from_square]) &&
                (abs(move.to_square - move.from_square) == 7 || abs(move.to_square - move.from_square) == 9) &&
                !(this->occupied & BB_SQUARES[move.to_square]));
    }

    bool Board::is_capture(const Move &move) const
    {
        /* Checks if the given pseudo-legal move is a capture. */
        Bitboard touched = BB_SQUARES[move.from_square] ^ BB_SQUARES[move.to_square];
        return bool(touched & this->occupied_co[!this->turn]) || this->is_en_passant(move);
    }

    bool Board::is_zeroing(const Move &move) const
    {
        /* Checks if the given pseudo-legal move is a capture or pawn move. */
        Bitboard touched = BB_SQUARES[move.from_square] ^ BB_SQUARES[move.to_square];
        return bool(touched & this->pawns || touched & this->occupied_co[!this->turn] || move.drop == PAWN);
    }

    bool Board::is_irreversible(const Move &move) const
    {
        /*
        Checks if the given pseudo-legal move is irreversible.

        In standard chess, pawn moves, captures, moves that destroy castling
        rights and moves that cede en passant are irreversible.

        This method has false-negatives with forced lines. For example, a check
        that will force the king to lose castling rights is not considered
        irreversible. Only the actual king move is.
        */
        return this->is_zeroing(move) || this->_reduces_castling_rights(move) || this->has_legal_en_passant();
    }

    bool Board::is_castling(const Move &move) const
    {
        /* Checks if the given pseudo-legal move is a castling move. */
        if (this->kings & BB_SQUARES[move.from_square])
        {
            int diff = square_file(move.from_square) - square_file(move.to_square);
            return abs(diff) > 1 || bool(this->rooks & this->occupied_co[this->turn] & BB_SQUARES[move.to_square]);
        }
        return false;
    }

    bool Board::is_kingside_castling(const Move &move) const
    {
        /*
        Checks if the given pseudo-legal move is a kingside castling move.
        */
        return this->is_castling(move) && square_file(move.to_square) > square_file(move.from_square);
    }

    bool Board::is_queenside_castling(const Move &move) const
    {
        /*
        Checks if the given pseudo-legal move is a queenside castling move.
        */
        return this->is_castling(move) && square_file(move.to_square) < square_file(move.from_square);
    }

    Bitboard Board::clean_castling_rights() const
    {
        /*
        Returns valid castling rights filtered from
        :data:`~Board::castling_rights`.
        */
        if (!this->_stack.empty())
        {
            // No new castling rights are assigned in a game, so we can assume
            // they were filtered already.
            return this->castling_rights;
        }

        Bitboard castling = this->castling_rights & this->rooks;
        Bitboard white_castling = castling & BB_RANK_1 & this->occupied_co[WHITE];
        Bitboard black_castling = castling & BB_RANK_8 & this->occupied_co[BLACK];

        if (!this->chess960)
        {
            // The rooks must be on a1, h1, a8 or h8.
            white_castling &= (BB_A1 | BB_H1);
            black_castling &= (BB_A8 | BB_H8);

            // The kings must be on e1 or e8.
            if (!(this->occupied_co[WHITE] & this->kings & ~this->promoted & BB_E1))
            {
                white_castling = 0;
            }
            if (!(this->occupied_co[BLACK] & this->kings & ~this->promoted & BB_E8))
            {
                black_castling = 0;
            }

            return white_castling | black_castling;
        }
        else
        {
            // The kings must be on the back rank.
            Bitboard white_king_mask = this->occupied_co[WHITE] & this->kings & BB_RANK_1 & ~this->promoted;
            Bitboard black_king_mask = this->occupied_co[BLACK] & this->kings & BB_RANK_8 & ~this->promoted;
            if (!white_king_mask)
            {
                white_castling = 0;
            }
            if (!black_king_mask)
            {
                black_castling = 0;
            }

            // There are only two ways of castling, a-side and h-side, and the
            // king must be between the rooks.
            Bitboard white_a_side = white_castling & -white_castling;
            Bitboard white_h_side = white_castling ? BB_SQUARES[msb(white_castling)] : 0;

            if (white_a_side && msb(white_a_side) > msb(white_king_mask))
            {
                white_a_side = 0;
            }
            if (white_h_side && msb(white_h_side) < msb(white_king_mask))
            {
                white_h_side = 0;
            }

            Bitboard black_a_side = (black_castling & -black_castling);
            Bitboard black_h_side = black_castling ? BB_SQUARES[msb(black_castling)] : BB_EMPTY;

            if (black_a_side && msb(black_a_side) > msb(black_king_mask))
            {
                black_a_side = 0;
            }
            if (black_h_side && msb(black_h_side) < msb(black_king_mask))
            {
                black_h_side = 0;
            }

            // Done.
            return black_a_side | black_h_side | white_a_side | white_h_side;
        }
    }

    bool Board::has_castling_rights(Color color) const
    {
        /* Checks if the given side has castling rights. */
        Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
        return bool(this->clean_castling_rights() & backrank);
    }

    bool Board::has_kingside_castling_rights(Color color) const
    {
        /*
        Checks if the given side has kingside (that is h-side in Chess960)
        castling rights.
        */
        Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
        Bitboard king_mask = this->kings & this->occupied_co[color] & backrank & ~this->promoted;
        if (!king_mask)
        {
            return false;
        }

        Bitboard castling_rights = this->clean_castling_rights() & backrank;
        while (castling_rights)
        {
            Bitboard rook = castling_rights & -castling_rights;

            if (rook > king_mask)
            {
                return true;
            }

            castling_rights = castling_rights & (castling_rights - 1);
        }

        return false;
    }

    bool Board::has_queenside_castling_rights(Color color) const
    {
        /*
        Checks if the given side has queenside (that is h-side in Chess960)
        castling rights.
        */
        Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
        Bitboard king_mask = this->kings & this->occupied_co[color] & backrank & ~this->promoted;
        if (!king_mask)
        {
            return false;
        }

        Bitboard castling_rights = this->clean_castling_rights() & backrank;
        while (castling_rights)
        {
            Bitboard rook = castling_rights & -castling_rights;

            if (rook < king_mask)
            {
                return true;
            }

            castling_rights = castling_rights & (castling_rights - 1);
        }

        return false;
    }

    bool Board::has_chess960_castling_rights()
    {
        /*
        Checks if there are castling rights that are only possible in Chess960.
        */
        // Get valid Chess960 castling rights.
        bool chess960 = this->chess960;
        this->chess960 = true;
        Bitboard castling_rights = this->clean_castling_rights();
        this->chess960 = chess960;

        // Standard chess castling rights can only be on the standard
        // starting rook squares.
        if (castling_rights & ~BB_CORNERS)
        {
            return true;
        }

        // If there are any castling rights in standard chess, the king must be
        // on e1 or e8.
        if (castling_rights & BB_RANK_1 && !(this->occupied_co[WHITE] & this->kings & BB_E1))
        {
            return true;
        }
        if (castling_rights & BB_RANK_8 && !(this->occupied_co[BLACK] & this->kings & BB_E8))
        {
            return true;
        }

        return false;
    }

    bool Board::is_valid() const
    {
        /*
        Gets a bitmask of possible problems with the position.

        :data:`~STATUS_VALID` if all basic validity requirements are met.
        This does not imply that the position is actually reachable with a
        series of legal moves from the starting position.

        Otherwise, bitwise combinations of:
        :data:`~STATUS_NO_WHITE_KING`,
        :data:`~STATUS_NO_BLACK_KING`,
        :data:`~STATUS_TOO_MANY_KINGS`,
        :data:`~STATUS_TOO_MANY_WHITE_PAWNS`,
        :data:`~STATUS_TOO_MANY_BLACK_PAWNS`,
        :data:`~STATUS_PAWNS_ON_BACKRANK`,
        :data:`~STATUS_TOO_MANY_WHITE_PIECES`,
        :data:`~STATUS_TOO_MANY_BLACK_PIECES`,
        :data:`~STATUS_BAD_CASTLING_RIGHTS`,
        :data:`~STATUS_INVALID_EP_SQUARE`,
        :data:`~STATUS_OPPOSITE_CHECK`,
        :data:`~STATUS_EMPTY`,
        :data:`~STATUS_RACE_CHECK`,
        :data:`~STATUS_RACE_OVER`,
        :data:`~STATUS_RACE_MATERIAL`,
        :data:`~STATUS_TOO_MANY_CHECKERS`,
        :data:`~STATUS_IMPOSSIBLE_CHECK`.
        */

        // There must be at least one piece.
        if (!this->occupied)
        {
            return false;
        }

        // There must be exactly one king of each color.
        if (!(this->occupied_co[WHITE] & this->kings))
        {
            return false;
        }
        if (!(this->occupied_co[BLACK] & this->kings))
        {
            return false;
        }
        if (popcount(this->occupied & this->kings) > 2)
        {
            return false;
        }

        // There can not be more than 16 pieces of any color.
        /* emitted - custom position
        if (popcount(this->occupied_co[WHITE]) > 16)
        {
            errors |= STATUS_TOO_MANY_WHITE_PIECES;
        }
        if (popcount(this->occupied_co[BLACK]) > 16)
        {
            errors |= STATUS_TOO_MANY_BLACK_PIECES;
        }
        */
        // There can not be more than 8 pawns of any color.
        /*emitted - custom position
        if (popcount(this->occupied_co[WHITE] & this->pawns) > 8)
        {
            errors |= STATUS_TOO_MANY_WHITE_PAWNS;
        }
        if (popcount(this->occupied_co[BLACK] & this->pawns) > 8)
        {
            errors |= STATUS_TOO_MANY_BLACK_PAWNS;
        }
        */
        // Pawns can not be on the back rank.
        if (this->pawns & BB_BACKRANKS)
        {
            return false;
        }

        // Castling rights.
        if (this->castling_rights != this->clean_castling_rights())
        {
            return false;
        }

        // En passant.
        std::optional<Square> valid_ep_square = this->_valid_ep_square();
        if (this->ep_square != valid_ep_square)
        {
            return false;
        }

        // Side to move giving check.
        if (this->was_into_check())
        {
            return false;
        }

        // More than the maximum number of possible checkers in the variant.
        Bitboard checkers = this->checkers_mask();
        Bitboard our_kings = this->kings & this->occupied_co[this->turn] & ~this->promoted;
        if (popcount(checkers) > 2)
        {
            return false;
        }
        else if (popcount(checkers) == 2 && ray(lsb(checkers), msb(checkers)) & our_kings)
        {
            return false;
        }
        else if (valid_ep_square != std::nullopt)
        {
            for (Square checker : scan_reversed(checkers))
            {
                if (ray(checker, *valid_ep_square) & our_kings)
                {
                    return false;
                }
            }
        }

        return true;
    }

    std::vector<Move> Board::generate_legal_moves(Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;

        Bitboard king_mask = this->kings & this->occupied_co[this->turn];
        if (king_mask)
        {
            Square king = msb(king_mask);
            Bitboard blockers = this->_slider_blockers(king);
            Bitboard checkers = this->attackers_mask(!this->turn, king);
            if (checkers)
            {
                for (const Move &move : this->_generate_evasions(king, checkers, from_mask, to_mask))
                {
                    if (this->_is_safe(king, blockers, move))
                    {
                        iter.push_back(move);
                    }
                }
            }
            else
            {
                for (const Move &move : this->generate_pseudo_legal_moves(from_mask, to_mask))
                {
                    if (this->_is_safe(king, blockers, move))
                    {
                        iter.push_back(move);
                    }
                }
            }
        }
        else
        {
            for (const Move &move : this->generate_pseudo_legal_moves(from_mask, to_mask))
            {
                iter.push_back(move);
            }
        }
        return iter;
    }

    std::vector<Move> Board::generate_legal_ep(Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;
        for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask))
        {
            if (!this->is_into_check(move))
            {
                iter.push_back(move);
            }
        }
        return iter;
    }

    std::vector<Move> Board::generate_legal_captures(Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;
        for (const Move &move : this->generate_legal_moves(from_mask, to_mask & this->occupied_co[!this->turn]))
        {
            iter.push_back(move);
        }
        for (const Move &move : this->generate_legal_ep(from_mask, to_mask))
        {
            iter.push_back(move);
        }
        return iter;
    }

    std::vector<Move> Board::generate_castling_moves(Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;
        Bitboard backrank = this->turn == WHITE ? BB_RANK_1 : BB_RANK_8;
        Bitboard king = this->occupied_co[this->turn] & this->kings & ~this->promoted & backrank & from_mask;
        king = king & -king;
        if (!king)
        {
            return iter;
        }

        Bitboard bb_c = BB_FILE_C & backrank;
        Bitboard bb_d = BB_FILE_D & backrank;
        Bitboard bb_f = BB_FILE_F & backrank;
        Bitboard bb_g = BB_FILE_G & backrank;

        for (Square candidate : scan_reversed(this->clean_castling_rights() & backrank & to_mask))
        {
            Bitboard rook = BB_SQUARES[candidate];

            bool a_side = rook < king;
            Bitboard king_to = a_side ? bb_c : bb_g;
            Bitboard rook_to = a_side ? bb_d : bb_f;

            Bitboard king_path = between(msb(king), msb(king_to));
            Bitboard rook_path = between(candidate, msb(rook_to));

            if (!((this->occupied ^ king ^ rook) & (king_path | rook_path | king_to | rook_to) ||
                  this->_attacked_for_king(king_path | king, this->occupied ^ king) ||
                  this->_attacked_for_king(king_to, this->occupied ^ king ^ rook ^ rook_to)))
            {
                iter.push_back(this->_from_chess960(this->chess960, msb(king), candidate));
            }
        }
        return iter;
    }

    bool Board::operator==(const Board &board) const
    {
        return (
            this->halfmove_clock == board.halfmove_clock &&
            this->fullmove_number == board.fullmove_number &&
            this->_transposition_key() == board._transposition_key());
    }

    Board Board::copy(std::variant<bool, int> stack) const
    {
        /*
        Creates a copy of the board.

        Defaults to copying the entire move stack. Alternatively, *stack* can
        be ``false``, or an integer to copy a limited number of moves.
        */
        BaseBoard board_copy = BaseBoard::copy();
        Board board = Board(STARTING_FEN, this->chess960);

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

        board.ep_square = this->ep_square;
        board.castling_rights = this->castling_rights;
        board.turn = this->turn;
        board.fullmove_number = this->fullmove_number;
        board.halfmove_clock = this->halfmove_clock;

        if (std::holds_alternative<bool>(stack) && std::get<bool>(stack) || std::holds_alternative<int>(stack) && std::get<int>(stack))
        {
            stack = int(std::holds_alternative<bool>(stack) && std::get<bool>(stack) ? this->move_stack.size() : std::get<int>(stack));
            board.move_stack = std::vector(std::end(this->move_stack) - std::get<int>(stack), std::end(this->move_stack));
            board._stack = std::vector(std::end(this->_stack) - std::get<int>(stack), std::end(this->_stack));
        }

        return board;
    }

    Board Board::empty(bool chess960)
    {
        /* Creates a new empty board. Also see :func:`~Board::clear()`. */
        return Board(std::nullopt, chess960);
    }

    std::tuple<Board, std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>>> Board::from_epd(const std::string &epd, bool chess960)
    {
        /*
        Creates a new board from an EPD string. See
        :func:`~Board::set_epd()`.

        Returns the board and the dictionary of parsed operations as a tuple.
        */
        Board board = Board::empty(chess960);
        return {board, board.set_epd(epd)};
    }

    bool Board::_is_halfmoves(int n) const
    {
        return this->halfmove_clock >= n && !this->generate_legal_moves().empty();
    }

    _BoardState Board::_board_state() const
    {
        return _BoardState(*this);
    }

    void Board::_set_castling_fen(const std::string &castling_fen)
    {
        if (castling_fen.empty() || castling_fen == "-")
        {
            this->castling_rights = BB_EMPTY;
            return;
        }

        if (!regex_match(castling_fen, FEN_CASTLING_REGEX))
        {
            throw std::invalid_argument("invalid castling fen: \"" + castling_fen + "\"");
        }

        this->castling_rights = BB_EMPTY;

        for (char flag : castling_fen)
        {
            Color color = std::isupper(flag) ? WHITE : BLACK;
            flag = std::tolower(flag);
            Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
            Bitboard rooks = this->occupied_co[color] & this->rooks & backrank;
            std::optional<Square> king = this->king(color);

            if (flag == 'q')
            {
                // Select the leftmost rook.
                if (king != std::nullopt && lsb(rooks) < *king)
                {
                    this->castling_rights |= rooks & -rooks;
                }
                else
                {
                    this->castling_rights |= BB_FILE_A & backrank;
                }
            }
            else if (flag == 'k')
            {
                // Select the rightmost rook.
                int rook = msb(rooks);
                if (king != std::nullopt && *king < rook)
                {
                    this->castling_rights |= BB_SQUARES[rook];
                }
                else
                {
                    this->castling_rights |= BB_FILE_H & backrank;
                }
            }
            else
            {
                auto it = std::find(std::begin(FILE_NAMES), std::end(FILE_NAMES), flag);
                if (it == std::end(FILE_NAMES))
                {
                    throw std::invalid_argument("");
                }
                this->castling_rights |= BB_FILES[std::distance(FILE_NAMES, it)] & backrank;
            }
        }
    }

    std::string Board::_epd_operations(const std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> &operations)
    {
        std::vector<char> epd;
        bool first_op = true;

        for (auto [opcode, operand] : operations)
        {
            if (opcode == "-")
            {
                throw std::runtime_error("dash (-) is not a valid epd opcode");
            }
            for (char blacklisted : {' ', '\n', '\t', '\r'})
            {
                if (opcode.find(blacklisted) != std::string::npos)
                {
                    throw std::runtime_error("invalid character ' ' in epd opcode: \"" + opcode + "\"");
                }
            }

            if (!first_op)
            {
                epd.push_back(' ');
            }
            first_op = false;
            epd.insert(std::end(epd), std::begin(opcode), std::end(opcode));

            if (std::holds_alternative<std::nullopt_t>(operand))
            {
                epd.push_back(';');
            }
            else if (std::holds_alternative<Move>(operand))
            {
                epd.push_back(' ');
                std::string san = this->san(std::get<Move>(operand));
                epd.insert(std::end(epd), std::begin(san), std::end(san));
                epd.push_back(';');
            }
            else if (std::holds_alternative<int>(operand))
            {
                std::string s = " " + std::to_string(std::get<int>(operand)) + ";";
                epd.insert(std::end(epd), std::begin(s), std::end(s));
            }
            else if (std::holds_alternative<float>(operand))
            {
                if (!std::isfinite(std::get<float>(operand)))
                {
                    throw std::runtime_error("expected numeric epd operand to be finite, got: " + std::to_string(std::get<float>(operand)));
                }
                std::string s = " " + std::to_string(std::get<float>(operand)) + ";";
                epd.insert(std::end(epd), std::begin(s), std::end(s));
            }
            else if (opcode == "pv" && std::holds_alternative<std::vector<Move>>(operand))
            {
                Board position = this->copy(false);
                for (const Move &move : std::get<std::vector<Move>>(operand))
                {
                    epd.push_back(' ');
                    std::string s = position.san_and_push(move);
                    epd.insert(std::end(epd), std::begin(s), std::end(s));
                }
                epd.push_back(';');
            }
            else if ((opcode == "am" || opcode == "bm") && std::holds_alternative<std::vector<Move>>(operand))
            {
                std::vector<std::string> v;
                for (const Move &move : std::get<std::vector<Move>>(operand))
                {
                    v.push_back(this->san(move));
                }
                sort(std::begin(v), std::end(v));
                for (const std::string &san : v)
                {
                    epd.push_back(' ');
                    epd.insert(std::end(epd), std::begin(san), std::end(san));
                }
                epd.push_back(';');
            }
            else
            {
                // Append as escaped string.
                std::string s = " \"";
                epd.insert(std::end(epd), std::begin(s), std::end(s));
                s = std::regex_replace(std::regex_replace(std::regex_replace(std::regex_replace(std::regex_replace(std::get<std::string>(operand), std::regex("\\"), "\\\\"), std::regex("\t"), "\\t"), std::regex("\r"), "\\r"), std::regex("\n"), "\\n"), std::regex("\""), "\\\"");
                epd.insert(std::end(epd), std::begin(s), std::end(s));
                s = "\";";
                epd.insert(std::end(epd), std::begin(s), std::end(s));
            }
        }

        return std::string(std::begin(epd), std::end(epd));
    }

    std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> Board::_parse_epd_ops(const std::string &operation_part, const std::function<Board()> &make_board) const
    {
        std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> operations;
        std::string state = "opcode";
        std::string opcode;
        std::string operand;
        std::optional<Board> position = std::nullopt;

        std::vector<std::optional<char>> v(std::begin(operation_part), std::end(operation_part));
        v.push_back(std::nullopt);
        for (std::optional<char> ch : v)
        {
            if (state == "opcode")
            {
                if (ch && (*ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n'))
                {
                    if (opcode == "-")
                    {
                        opcode = "";
                    }
                    else if (!opcode.empty())
                    {
                        state = "after_opcode";
                    }
                }
                else if (ch == std::nullopt || *ch == ';')
                {
                    if (opcode == "-")
                    {
                        opcode = "";
                    }
                    else if (!opcode.empty())
                    {
                        operations.insert_or_assign(opcode, opcode == "pv" || opcode == "am" || opcode == "bm" ? std::vector<Move>() : std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>(std::nullopt));
                        opcode = "";
                    }
                }
                else
                {
                    opcode += *ch;
                }
            }
            else if (state == "after_opcode")
            {
                if (ch && (*ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n'))
                {
                }
                else if (*ch == '\"')
                {
                    state = "string";
                }
                else if (ch == std::nullopt || *ch == ';')
                {
                    if (!opcode.empty())
                    {
                        operations.insert_or_assign(opcode, opcode == "pv" || opcode == "am" || opcode == "bm" ? std::vector<Move>() : std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>(std::nullopt));
                        opcode = "";
                    }
                    state = "opcode";
                }
                else if (*ch == '+' || *ch == '-' || *ch == '.' || isdigit(*ch))
                {
                    operand = *ch;
                    state = "numeric";
                }
                else
                {
                    operand = *ch;
                    state = "san";
                }
            }
            else if (state == "numeric")
            {
                if (ch == std::nullopt || *ch == ';')
                {
                    if (operand.find('.') != std::string::npos || operand.find('e') != std::string::npos || operand.find('E') != std::string::npos)
                    {
                        float parsed = stof(operand);
                        if (!std::isfinite(parsed))
                        {
                            throw std::invalid_argument("invalid numeric operand for epd operation \"" + opcode + "\": \"" + operand + "\"");
                        }
                        operations.insert_or_assign(opcode, parsed);
                    }
                    else
                    {
                        operations.insert_or_assign(opcode, stoi(operand));
                    }
                    opcode = "";
                    operand = "";
                    state = "opcode";
                }
                else
                {
                    operand += *ch;
                }
            }
            else if (state == "string")
            {
                if (ch == std::nullopt || *ch == '\"')
                {
                    operations.insert_or_assign(opcode, operand);
                    opcode = "";
                    operand = "";
                    state = "opcode";
                }
                else if (*ch == '\\')
                {
                    state = "string_escape";
                }
                else
                {
                    operand += *ch;
                }
            }
            else if (state == "string_escape")
            {
                if (ch == std::nullopt)
                {
                    operations.insert_or_assign(opcode, operand);
                    opcode = "";
                    operand = "";
                    state = "opcode";
                }
                else if (*ch == 'r')
                {
                    operand += "\r";
                    state = "string";
                }
                else if (*ch == 'n')
                {
                    operand += "\n";
                    state = "string";
                }
                else if (*ch == 't')
                {
                    operand += "\t";
                    state = "string";
                }
                else
                {
                    operand += *ch;
                    state = "string";
                }
            }
            else if (state == "san")
            {
                if (ch == std::nullopt || *ch == ';')
                {
                    if (position == std::nullopt)
                    {
                        position = make_board();
                    }

                    if (opcode == "pv")
                    {
                        // A variation.
                        std::vector<Move> variation;
                        std::istringstream iss(operand);
                        std::vector<std::string> split = {std::istream_iterator<std::string>(iss), {}};
                        for (const std::string &token : split)
                        {
                            Move move = position->parse_xboard(token);
                            variation.push_back(move);
                            position->push(move);
                        }

                        // Reset the position.
                        while (!position->move_stack.empty())
                        {
                            position->pop();
                        }

                        operations.insert_or_assign(opcode, variation);
                    }
                    else if (opcode == "bm" || opcode == "am")
                    {
                        // A set of moves.
                        std::istringstream iss(operand);
                        std::vector<std::string> split = {std::istream_iterator<std::string>(iss), {}};
                        std::vector<Move> parsed;
                        for (const std::string &token : split)
                        {
                            parsed.push_back(position->parse_xboard(token));
                        }
                        operations.insert_or_assign(opcode, parsed);
                    }
                    else
                    {
                        // A single move.
                        operations.insert_or_assign(opcode, position->parse_xboard(operand));
                    }

                    opcode = "";
                    operand = "";
                    state = "opcode";
                }
                else
                {
                    operand += *ch;
                }
            }
        }

        if (state != "opcode")
        {
            throw;
        }
        return operations;
    }

    std::string Board::_algebraic(const Move &move, bool long_)
    {
        std::string san = this->_algebraic_and_push(move, long_);
        this->pop();
        return san;
    }

    std::string Board::_algebraic_and_push(const Move &move, bool long_)
    {
        std::string san = this->_algebraic_without_suffix(move, long_);

        // Look ahead for check or checkmate.
        this->push(move);
        bool is_check = this->is_check();
        bool is_checkmate = (is_check && this->is_checkmate());

        // Add check or checkmate suffix.
        if (is_checkmate && move)
        {
            return san + "#";
        }
        else if (is_check && move)
        {
            return san + "+";
        }
        else
        {
            return san;
        }
    }

    std::string Board::_algebraic_without_suffix(const Move &move, bool long_)
    {
        // Null move.
        if (!move)
        {
            return "--";
        }

        // Drops.
        std::string san;
        if (move.drop)
        {
            if (*move.drop != PAWN)
            {
                san = std::toupper(piece_symbol(*move.drop));
            }
            san += "@" + SQUARE_NAMES[move.to_square];
            return san;
        }

        // Castling.
        if (this->is_castling(move))
        {
            if (square_file(move.to_square) < square_file(move.from_square))
            {
                return "O-O-O";
            }
            else
            {
                return "O-O";
            }
        }

        std::optional<PieceType> piece_type = this->piece_type_at(move.from_square);
        if (!piece_type)
        {
            throw std::runtime_error("san() and lan() expect move to be legal or null, but got " + std::string(move) + " in " + this->fen());
        }
        bool capture = this->is_capture(move);

        if (*piece_type != PAWN)
        {
            san = std::toupper(piece_symbol(*piece_type));
        }

        if (long_)
        {
            san += SQUARE_NAMES[move.from_square];
        }
        else if (*piece_type != PAWN)
        {
            // Get ambiguous move candidates.
            // Relevant candidates: not exactly the current move,
            // but to the same square.
            Bitboard others = 0;
            Bitboard from_mask = this->pieces_mask(*piece_type, this->turn);
            from_mask &= ~BB_SQUARES[move.from_square];
            Bitboard to_mask = BB_SQUARES[move.to_square];
            for (const Move &candidate : this->generate_legal_moves(from_mask, to_mask))
            {
                others |= BB_SQUARES[candidate.from_square];
            }

            // Disambiguate.
            if (others)
            {
                bool row = false, column = false;

                if (others & BB_RANKS[square_rank(move.from_square)])
                {
                    column = true;
                }

                if (others & BB_FILES[square_file(move.from_square)])
                {
                    row = true;
                }
                else
                {
                    column = true;
                }

                if (column)
                {
                    san += FILE_NAMES[square_file(move.from_square)];
                }
                if (row)
                {
                    san += RANK_NAMES[square_rank(move.from_square)];
                }
            }
        }
        else if (capture)
        {
            san += FILE_NAMES[square_file(move.from_square)];
        }

        // Captures.
        if (capture)
        {
            san += "x";
        }
        else if (long_)
        {
            san += "-";
        }

        // Destination square.
        san += SQUARE_NAMES[move.to_square];

        // Promotion.
        if (move.promotion)
        {
            san += "=" + std::string(1, std::toupper(piece_symbol(*move.promotion)));
        }

        return san;
    }

    bool Board::_reduces_castling_rights(const Move &move) const
    {
        Bitboard cr = this->clean_castling_rights();
        Bitboard touched = BB_SQUARES[move.from_square] ^ BB_SQUARES[move.to_square];
        return bool(touched & cr ||
                    cr & BB_RANK_1 && touched & this->kings & this->occupied_co[WHITE] & ~this->promoted ||
                    cr & BB_RANK_8 && touched & this->kings & this->occupied_co[BLACK] & ~this->promoted);
    }

    std::optional<Square> Board::_valid_ep_square() const
    {
        if (!this->ep_square)
        {
            return std::nullopt;
        }

        int ep_rank;
        Bitboard pawn_mask, seventh_rank_mask;
        if (this->turn == WHITE)
        {
            ep_rank = 5;
            pawn_mask = shift_down(BB_SQUARES[*this->ep_square]);
            seventh_rank_mask = shift_up(BB_SQUARES[*this->ep_square]);
        }
        else
        {
            ep_rank = 2;
            pawn_mask = shift_up(BB_SQUARES[*this->ep_square]);
            seventh_rank_mask = shift_down(BB_SQUARES[*this->ep_square]);
        }

        // The en passant square must be on the third or sixth rank.
        if (square_rank(*this->ep_square) != ep_rank)
        {
            return std::nullopt;
        }

        // The last move must have been a double pawn push, so there must
        // be a pawn of the correct color on the fourth or fifth rank.
        if (!(this->pawns & this->occupied_co[!this->turn] & pawn_mask))
        {
            return std::nullopt;
        }

        // And the en passant square must be empty.
        if (this->occupied & BB_SQUARES[*this->ep_square])
        {
            return std::nullopt;
        }

        // And the second rank must be empty.
        if (this->occupied & seventh_rank_mask)
        {
            return std::nullopt;
        }

        return this->ep_square;
    }

    bool Board::_ep_skewered(Square king, Square capturer) const
    {
        // Handle the special case where the king would be in check if the
        // pawn and its capturer disappear from the rank.

        // Vertical skewers of the captured pawn are not possible. (Pins on
        // the capturer are not handled here.)
        if (this->ep_square == std::nullopt)
        {
            throw;
        }

        Square last_double = *this->ep_square + (this->turn == WHITE ? -8 : 8);

        Bitboard occupancy = (this->occupied & ~BB_SQUARES[last_double] &
                                  ~BB_SQUARES[capturer] |
                              BB_SQUARES[*this->ep_square]);

        // Horizontal attack on the fifth or fourth rank.
        Bitboard horizontal_attackers = this->occupied_co[!this->turn] & (this->rooks | this->queens);
        if (BB_RANK_ATTACKS[king].at(BB_RANK_MASKS[king] & occupancy) & horizontal_attackers)
        {
            return true;
        }

        // Diagonal skewers. These are not actually possible in a real game,
        // because if the latest double pawn move covers a diagonal attack,
        // then the other side would have been in check already.
        Bitboard diagonal_attackers = this->occupied_co[!this->turn] & (this->bishops | this->queens);
        if (BB_DIAG_ATTACKS[king].at(BB_DIAG_MASKS[king] & occupancy) & diagonal_attackers)
        {
            return true;
        }

        return false;
    }

    Bitboard Board::_slider_blockers(Square king) const
    {
        Bitboard rooks_and_queens = this->rooks | this->queens;
        Bitboard bishops_and_queens = this->bishops | this->queens;

        Bitboard snipers = ((BB_RANK_ATTACKS[king].at(0) & rooks_and_queens) |
                            (BB_FILE_ATTACKS[king].at(0) & rooks_and_queens) |
                            (BB_DIAG_ATTACKS[king].at(0) & bishops_and_queens));

        Bitboard blockers = 0;

        for (Square sniper : scan_reversed(snipers & this->occupied_co[!this->turn]))
        {
            Bitboard b = between(king, sniper) & this->occupied;

            // Add to blockers if exactly one piece in-between.
            if (b && BB_SQUARES[msb(b)] == b)
            {
                blockers |= b;
            }
        }

        return blockers & this->occupied_co[this->turn];
    }

    bool Board::_is_safe(Square king, Bitboard blockers, const Move &move) const
    {
        if (move.from_square == king)
        {
            if (this->is_castling(move))
            {
                return true;
            }
            else
            {
                return !this->is_attacked_by(!this->turn, move.to_square);
            }
        }
        else if (this->is_en_passant(move))
        {
            return bool(this->pin_mask(this->turn, move.from_square) & BB_SQUARES[move.to_square] &&
                        !this->_ep_skewered(king, move.from_square));
        }
        else
        {
            return bool(!(blockers & BB_SQUARES[move.from_square]) ||
                        ray(move.from_square, move.to_square) & BB_SQUARES[king]);
        }
    }

    std::vector<Move> Board::_generate_evasions(Square king, Bitboard checkers, Bitboard from_mask, Bitboard to_mask) const
    {
        std::vector<Move> iter;
        Bitboard sliders = checkers & (this->bishops | this->rooks | this->queens);

        Bitboard attacked = 0;
        for (Square checker : scan_reversed(sliders))
        {
            attacked |= ray(king, checker) & ~BB_SQUARES[checker];
        }

        if (BB_SQUARES[king] & from_mask)
        {
            for (Square to_square : scan_reversed(BB_KING_ATTACKS[king] & ~this->occupied_co[this->turn] & ~attacked & to_mask))
            {
                iter.push_back(Move(king, to_square));
            }
        }

        Square checker = msb(checkers);
        if (BB_SQUARES[checker] == checkers)
        {
            // Capture or block a single checker.
            Bitboard target = between(king, checker) | checkers;

            for (const Move &move : this->generate_pseudo_legal_moves(~this->kings & from_mask, target & to_mask))
            {
                iter.push_back(move);
            }

            // Capture the checking pawn en passant (but avoid yielding
            // duplicate moves).
            if (this->ep_square && !(BB_SQUARES[*this->ep_square] & target))
            {
                Square last_double = *this->ep_square + (this->turn == WHITE ? -8 : 8);
                if (last_double == checker)
                {
                    for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask))
                    {
                        iter.push_back(move);
                    }
                }
            }
        }
        return iter;
    }

    bool Board::_attacked_for_king(Bitboard path, Bitboard occupied) const
    {
        for (Square sq : scan_reversed(path))
        {
            if (this->_attackers_mask(!this->turn, sq, occupied))
            {
                return true;
            }
        }
        return false;
    }

    Move Board::_from_chess960(bool chess960, Square from_square, Square to_square, std::optional<PieceType> promotion, std::optional<PieceType> drop) const
    {
        if (!chess960 && promotion == std::nullopt && drop == std::nullopt)
        {
            if (from_square == E1 && this->kings & BB_E1)
            {
                if (to_square == H1)
                {
                    return Move(E1, G1);
                }
                else if (to_square == A1)
                {
                    return Move(E1, C1);
                }
            }
            else if (from_square == E8 && this->kings & BB_E8)
            {
                if (to_square == H8)
                {
                    return Move(E8, G8);
                }
                else if (to_square == A8)
                {
                    return Move(E8, C8);
                }
            }
        }

        return Move(from_square, to_square, promotion, drop);
    }

    Move Board::_to_chess960(const Move &move) const
    {
        if (move.from_square == E1 && this->kings & BB_E1)
        {
            if (move.to_square == G1 && !(this->rooks & BB_G1))
            {
                return Move(E1, H1);
            }
            else if (move.to_square == C1 && !(this->rooks & BB_C1))
            {
                return Move(E1, A1);
            }
        }
        else if (move.from_square == E8 && this->kings & BB_E8)
        {
            if (move.to_square == G8 && !(this->rooks & BB_G8))
            {
                return Move(E8, H8);
            }
            else if (move.to_square == C8 && !(this->rooks & BB_C8))
            {
                return Move(E8, A8);
            }
        }

        return move;
    }

    std::tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, Square> Board::_transposition_key() const
    {
        return std::make_tuple(this->pawns, this->knights, this->bishops, this->rooks,
                               this->queens, this->kings,
                               this->occupied_co[WHITE], this->occupied_co[BLACK],
                               this->turn, this->clean_castling_rights(),
                               this->has_legal_en_passant() ? *this->ep_square : 64);
    }

    std::ostream &operator<<(std::ostream &os, Board board)
    {
        if (!board.chess960)
        {
            os << "Board(\"" << board.fen() << "\")";
        }
        else
        {
            os << "Board(\"" << board.fen() << "\", chess960=true)";
        }
        return os;
    }

