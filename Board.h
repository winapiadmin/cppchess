#ifndef BOARD_H
#define BOARD_H
#include "BaseBoard.h"
    class Board;

    class _BoardState
    {

    public:
        Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_w, occupied_b, occupied, promoted;
        Color turn;
        Bitboard castling_rights;
        std::optional<Square> ep_square;
        int halfmove_clock, fullmove_number;
        _BoardState(const Board &);

        void restore(Board &) const;
    };


    class Board : public BaseBoard
    {
        /*
        A :class:`~chess::BaseBoard`, additional information representing
        a chess position, and a :data:`move stack <chess::Board::move_stack>`.

        Provides :func:`move generation <chess::Board::legal_moves()>`, validation,
        :func:`parsing <chess::Board::parse_san()>`, attack generation,
        :func:`game end detection <chess::Board::is_game_over()>`,
        and the capability to :func:`make <chess::Board::push()>` and
        :func:`unmake <chess::Board::pop()>` moves.

        The board is initialized to the standard chess starting position,
        unless otherwise specified in the optional *fen* argument.
        If *fen* is ``std::nullopt``, an empty board is created.

        Optionally supports *chess960*. In Chess960, castling moves are encoded
        by a king move to the corresponding rook square.
        Use :func:`chess::Board::from_chess960_pos()` to create a board with one
        of the Chess960 starting positions.

        It's safe to set :data:`~Board::turn`, :data:`~Board::castling_rights`,
        :data:`~Board::ep_square`, :data:`~Board::halfmove_clock` and
        :data:`~Board::fullmove_number` directly.

        .. warning::
            It is possible to set up and work with invalid positions. In this
            case, :class:`~chess::Board` implements a kind of "pseudo-chess"
            (useful to gracefully handle errors or to implement chess variants).
            Use :func:`~chess::Board::is_valid()` to detect invalid positions.
        */

    public:
        static std::optional<std::string> uci_variant;
        static std::optional<std::string> xboard_variant;
        static std::string starting_fen;

        Color turn=WHITE;
        /* The side to move (``chess::WHITE`` or ``chess::BLACK``). */

        Bitboard castling_rights=0;
        /*
        Bitmask of the rooks with castling rights.

        To test for specific squares:

        >>> #include "chess.cpp"
        >>> #include <iostream>
        >>>
        >>> chess::Board board;
        >>> std::cout << bool(board.castling_rights & chess::BB_H1);  // White can castle with the h1 rook
        1

        To add a specific square:

        >>> board.castling_rights |= chess::BB_A1;

        Use :func:`~chess::Board::set_castling_fen()` to set multiple castling
        rights. Also see :func:`~chess::Board::has_castling_rights()`,
        :func:`~chess::Board::has_kingside_castling_rights()`,
        :func:`~chess::Board::has_queenside_castling_rights()`,
        :func:`~chess::Board::has_chess960_castling_rights()`,
        :func:`~chess::Board::clean_castling_rights()`.
        */

        std::optional<Square> ep_square=std::nullopt;
        /*
        The potential en passant square on the third or sixth rank or ``std::nullopt``.

        Use :func:`~chess::Board::has_legal_en_passant()` to test if en passant
        capturing would actually be possible on the next move.
        */

        int fullmove_number=1;
        /*
        Counts move pairs. Starts at `1` and is incremented after every move
        of the black side.
        */

        int halfmove_clock=0;
        /* The number of half-moves since the last capture or pawn move. */

        bool chess960=false;
        /*
        Whether the board is in Chess960 mode. In Chess960 castling moves are
        represented as king moves to the corresponding rook square.
        */

        std::vector<Move> move_stack;
        /*
        The move stack. Use :func:`Board::push() <chess::Board::push()>`,
        :func:`Board::pop() <chess::Board::pop()>`,
        :func:`Board::peek() <chess::Board::peek()>` and
        :func:`Board::clear_stack() <chess::Board::clear_stack()>` for
        manipulation.
        */

        Board(const std::optional<std::string> & = STARTING_FEN, bool = false);

        void reset();

        void reset_board();

        void clear();

        void clear_board();

        void clear_stack();

        Board root() const;

        int ply() const;

        std::optional<Piece> remove_piece_at(Square);

        void set_piece_at(Square, const std::optional<Piece> &, bool = false);

        std::vector<Move> generate_pseudo_legal_moves(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_pseudo_legal_ep(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_pseudo_legal_captures(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        Bitboard checkers_mask() const;

        bool is_check() const;

        bool gives_check(const Move &);

        bool is_into_check(const Move &) const;

        bool was_into_check() const;

        bool is_pseudo_legal(Move) const;

        bool is_legal(const Move &) const;

        bool is_game_over(bool = false);

        std::string result(bool = false);

        bool is_checkmate() const;

        bool is_stalemate() const;

        bool is_insufficient_material() const;

        bool has_insufficient_material(Color) const;

        bool is_seventyfive_moves() const;

        bool is_fivefold_repetition();

        bool can_claim_draw();

        bool is_fifty_moves() const;

        bool can_claim_fifty_moves();

        bool can_claim_threefold_repetition();

        bool is_repetition(int = 3);

        void push(Move);

        Move pop();

        Move peek() const;

        Move find_move(Square, Square, std::optional<PieceType> = std::nullopt);

        std::string castling_shredder_fen() const;

        std::string castling_xfen() const;

        bool has_pseudo_legal_en_passant() const;

        bool has_legal_en_passant() const;

        std::string fen(bool = false, _EnPassantSpec = "legal", std::optional<bool> = std::nullopt);

        std::string shredder_fen(_EnPassantSpec = "legal", std::optional<bool> = std::nullopt);

        void set_fen(const std::string &);

        void set_castling_fen(const std::string &);

        void set_board_fen(const std::string &);

        void set_piece_map(const std::unordered_map<Square, Piece> &);

        std::string epd(bool = false, const _EnPassantSpec & = "legal", std::optional<bool> = std::nullopt, const std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> & = {});

        std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> set_epd(const std::string &);

        std::string san(const Move &);

        std::string lan(const Move &);

        std::string san_and_push(const Move &);

        std::string variation_san(const std::vector<Move> &) const;

        Move parse_san(const std::string &);

        Move push_san(const std::string &);

        std::string uci(Move, std::optional<bool> = std::nullopt) const;

        Move parse_uci(const std::string &);

        Move push_uci(const std::string &);

        std::string xboard(const Move &, std::optional<bool> = std::nullopt) const;

        Move parse_xboard(const std::string &);

        Move push_xboard(const std::string &);

        bool is_en_passant(const Move &) const;

        bool is_capture(const Move &) const;

        bool is_zeroing(const Move &) const;

        bool is_irreversible(const Move &) const;

        bool is_castling(const Move &) const;

        bool is_kingside_castling(const Move &) const;

        bool is_queenside_castling(const Move &) const;

        Bitboard clean_castling_rights() const;

        bool has_castling_rights(Color) const;

        bool has_kingside_castling_rights(Color) const;

        bool has_queenside_castling_rights(Color) const;

        bool has_chess960_castling_rights();

        bool is_valid() const;

        std::vector<Move> generate_legal_moves(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_legal_ep(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_legal_captures(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_castling_moves(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        bool operator==(const Board &) const;
        Board copy(std::variant<bool, int> = true) const;

        static Board empty(bool = false);

        static std::tuple<Board, std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>>> from_epd(const std::string &, bool = false);

        static Board from_chess960_pos(int);

    private:
        std::vector<_BoardState> _stack;

        bool _is_halfmoves(int) const;

        _BoardState _board_state() const;

        void _push_capture(const Move &, Square, PieceType, bool) const;

        void _set_castling_fen(const std::string &);

        std::string _epd_operations(const std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> &);

        std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> _parse_epd_ops(const std::string &, const std::function<Board()> &) const;

        std::string _algebraic(const Move &, bool = false);

        std::string _algebraic_and_push(const Move &, bool = false);

        std::string _algebraic_without_suffix(const Move &, bool = false);

        bool _reduces_castling_rights(const Move &) const;

        std::optional<Square> _valid_ep_square() const;

        bool _ep_skewered(Square, Square) const;

        Bitboard _slider_blockers(Square) const;

        bool _is_safe(Square, Bitboard, const Move &) const;

        std::vector<Move> _generate_evasions(Square, Bitboard, Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        bool _attacked_for_king(Bitboard, Bitboard) const;

        Move _from_chess960(bool, Square, Square, std::optional<PieceType> = std::nullopt, std::optional<PieceType> = std::nullopt) const;

        Move _to_chess960(const Move &) const;

        std::tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, Square> _transposition_key() const;
    };
    std::ostream &operator<<(std::ostream &, Board);
#endif // BOARD_H
