#include "eval.h"
Score Fianchetto(Board board, Color pov)
{
    return popcount(board.pieces_mask(BISHOP, pov) & (0x4200 << 40*(!pov)));
}
Score BishopPair(Board board, Color color)
{
    return !board.pieces_mask(BISHOP, color) %2 * 12;
}
bool is_bad_bishop(Board board, Square square)
{
    optional<Piece> piece = board.piece_at(square);
    if (piece && piece.value().piece_type == BISHOP)
    {
        // Get the color of the bishop
        Color color = piece.value().color;
        // Get the central pawns (squares in front of the bishop)
        std::vector<Square> central_pawns = {square + SQUARES[55],square + SQUARES[47]};
        for (Square pawn_square : central_pawns)
        {
            if (board.piece_at(pawn_square) && board.piece_at(pawn_square).value().color == color)
            {
                return true;
            }
        }
    }
    return false;
}
Board colorflip(Board board)
{
    return board.mirror();
}
Score space_area(Board pos, Square sq)
{
    int v = 0;
    int rank = square_rank(sq);
    int file = square_file(sq);

    if ((rank >= 2 && rank <= 4 && file >= 3 && file <= 6)
            && (pos.piece_at(sq).value_or(Piece(PAWN, BLACK)).symbol() != 'P')
            && (pos.piece_at(square(file-1, rank-1)).value_or(Piece(PAWN, WHITE)).symbol() != 'p')
            && (pos.piece_at(square(file + 1, rank - 1)).value_or(Piece(PAWN, WHITE)).symbol() != 'p'))
    {
        v++;

        if ((pos.piece_at(square(file, rank - 1)).value_or(Piece(PAWN, BLACK)).symbol() == 'P' ||
                pos.piece_at(square(file, rank - 2)).value_or(Piece(PAWN, BLACK)).symbol() == 'P'     ||
                pos.piece_at(square(file, rank - 3)).value_or(Piece(PAWN, BLACK)).symbol() == 'P')
                && !popcount(colorflip(pos).attacks_mask(square(file, 7-rank))))
        {
            v++;
        }
    }

    return v;
}
Score piece_value_bonus(Board pos, Square square, bool mg)
{
    int a[2][5]=  {{124, 781, 825, 1276, 2538},{206, 854, 915, 1380, 2682}};
    int i = pos.piece_type_at(square).value_or(-1)-1;
    if (i >= 0) return a[mg][i];
    return 0;
}
Score non_pawn_material(Board pos)
{
    int total=0;
    for (Square square:SQUARES)
    {
        int i = pos.piece_type_at(square).value_or(0)-1+pos.color_at(square).value_or(-1)-1;
        if (i >= 0&& i < 5) total += piece_value_bonus(pos, square, true);
    }

    return 0;
}
Score space(Board board)
{
    if (non_pawn_material(board) + non_pawn_material(colorflip(board)) < 12222) return 0;
    int pieceCount =popcount(board.occupied_co[WHITE]), blockedCount=0;
    for (int x = 0; x < 8; x++)
    {
        for (int y=0; y < 8; y++)
        {
            if (board.piece_at(square(x,y)).value_or(Piece(PAWN, BLACK)).symbol()=='P' &&
                    board.piece_at(square(x,y-1)).value_or(Piece(PAWN, WHITE)).symbol()=='p'||
                    board.piece_at(square(x-1,y-2)).value_or(Piece(PAWN, WHITE)).symbol()=='p'&&
                    board.piece_at(square(x,y-1)).value_or(Piece(PAWN, WHITE)).symbol()=='p')blockedCount++;
            if (board.piece_at(square(x,y)).value_or(Piece(PAWN, WHITE)).symbol()=='p' &&
                    board.piece_at(square(x,y-1)).value_or(Piece(PAWN, WHITE)).symbol()=='P'||
                    board.piece_at(square(x-1,y-2)).value_or(Piece(PAWN, WHITE)).symbol()=='P'&&
                    board.piece_at(square(x,y-1)).value_or(Piece(PAWN, WHITE)).symbol()=='P')blockedCount++;
        }
    }
    int weight = pieceCount - 3 + min(blockedCount, 9);
    int total=0;
    for (Square sq:SQUARES)
    {
        total += space_area(board, sq);
    }
    return (total * weight * weight / 16);
}
int phase(Board pos)
{
    int npm = non_pawn_material(pos) + non_pawn_material(colorflip(pos));
    npm = max(EndgameLimit, min(npm, MidgameLimit));
    return (((npm - EndgameLimit) * 128) / (MidgameLimit - EndgameLimit));
}
std::vector<Square> get_pawn_positions(const Board& board, Color color)
{
    std::vector<Square> pawns;
    for (Square square:scan_reversed(board.pieces_mask(PAWN,color)))
    {
        pawns.push_back(square);
    }
    return pawns;
}

bool is_isolated_pawn(const Board& board, Square sq, Color color)
{
    int file = square_file(sq);
    std::vector<int> files_to_check = { file - 1, file + 1 };
    for (int f : files_to_check)
    {
        if (f >= 0 && f < 8)
        {
            for (int rank = 0; rank < 8; ++rank)
            {
                if (board.piece_at(square(f, rank)) && board.piece_at(square(f, rank)).value() == Piece(PAWN, color))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool is_doubled_pawn(const Board& board, int file, Color color)
{
    int count = 0;
    for (int rank = 0; rank < 8; ++rank)
    {
        if (board.piece_at(square(file, rank)) && board.piece_at(square(file, rank)).value() == Piece(PAWN, color))
        {
            count++;
        }
    }
    return count > 1;
}

int evaluate_pawn_structure(const Board& board)
{
    int score = 0;
    std::vector<Square> white_pawns = get_pawn_positions(board, WHITE);
    std::vector<Square> black_pawns = get_pawn_positions(board, BLACK);

    // Evaluate white pawns
    for (Square pawn : white_pawns)
    {
        int file = square_file(pawn);
        if (square_rank(pawn) > 4 && !(board.pieces_mask(PAWN, BLACK) & BB_FILES[file])) score += PawnWt[0]+PassedRank[square_rank(pawn)]+PSQT[0][PAWN][pawn];
        if (is_isolated_pawn(board, pawn, WHITE))
        {
            score -= PawnWt[2]; // Penalize isolated pawns
        }
        if (is_doubled_pawn(board, file, WHITE))
        {
            score -= PawnWt[1]; // Penalize doubled pawns
        }
    }

    // Evaluate black pawns
    for (Square pawn : black_pawns)
    {
        int file = square_file(pawn);
        if (square_rank(pawn) > 4 && !(board.pieces_mask(PAWN, BLACK) & BB_FILES[file])) score -= PawnWt[0]+PassedRank[square_rank(pawn)]+PSQT[0][PAWN][pawn];
        if (is_isolated_pawn(board, pawn, BLACK))
        {
            score += PawnWt[2]; // Penalize isolated pawns
        }
        if (is_doubled_pawn(board, file, BLACK))
        {
            score += PawnWt[1]; // Penalize doubled pawns
        }
    }

    return score;
}
bool is_trapped(Board &board,Square square, Color opponent)
{
    if ((board.occupied&(square))&&board.is_attacked_by(opponent, square))
    {
        Board board2=board.copy();
        for (Move x:board.generate_legal_moves(1 << square, BB_ALL))
        {
            board2.push(x);
            if (!board2.is_attacked_by(opponent, x.to_square)) return false;
            board2.pop();
        }
        return true;
    }
    return false;
}
inline bool is_on_semiopen_file(Board &pos,Color c, Square s)
{
    return !(pos.pieces_mask(PAWN,c) & BB_FILES[s]);
}
inline Score long_diagonal_bishop(Board &board)
{
    return popcount(board.pieces_mask(BISHOP,board.turn)&(1 << 9|1 << 13))*LongDiagonalBishop-popcount(board.pieces_mask(BISHOP,!board.turn)&(1 << 9|1 << 13))*LongDiagonalBishop;
}
bool weak_queen_protection(Board &board, Color q)
{

    Bitboard queens = board.pieces_mask(QUEEN, q);

    for (Square queen_square:scan_reversed(queens))
    {
        Bitboard attackers = board.attackers_mask(!q, queen_square);
        Bitboard defenders = board.attackers_mask(q, queen_square);

        if (popcount(attackers) > popcount(defenders))
            return true;
    }
    return false;
}
int eval(Board board)
{
    //endgame:
    //board.pieces_mask(PAWN, WHITE) & BB_RANK_5
    //board.pieces_mask(PAWN, BLACK) & BB_RANK_4
    Score P = 100,
          N = 320,
          B = 330,
          R = 500,
          Q = 900,
          K = VALUE_INFINITE;
    vector<int> count=
    {
        popcount(board.pieces_mask(PAWN, WHITE))-popcount(board.pieces_mask(PAWN, BLACK)),
        popcount(board.pieces_mask(KNIGHT, WHITE))-popcount(board.pieces_mask(KNIGHT, BLACK)),
        popcount(board.pieces_mask(BISHOP, WHITE))-popcount(board.pieces_mask(BISHOP, BLACK)),
        popcount(board.pieces_mask(ROOK, WHITE))-popcount(board.pieces_mask(ROOK, BLACK)),
        popcount(board.pieces_mask(QUEEN, WHITE))-popcount(board.pieces_mask(QUEEN, BLACK)),
        popcount(board.pieces_mask(KING, WHITE))-popcount(board.pieces_mask(KING, BLACK))
    };
    int evalu=count[0]*P+count[1]*N+count[2]*B+count[3]*R+count[4]*Q+count[5]*K;
    // Control
    // Control is closely associated with "Space"
    // We will calculate for every square the delta of white attackers to black attackers and sum the deltas
    int control, whitecontrol = 0, blackcontrol= 0;
    for (Square spacesquare: SQUARES)
    {
        whitecontrol += popcount(board.attackers_mask(WHITE, spacesquare));
        blackcontrol += popcount(board.attackers_mask(BLACK, spacesquare));
    }
    control = whitecontrol - blackcontrol;
    for (Square c:scan_reversed(board.occupied_co[board.turn]))
    {
        optional<Piece> piece = board.piece_at(c);
        if (piece)
            evalu += pesto_table[phase(board)==0][piece.value().piece_type][c]/10;
    }
    for (Square c:scan_reversed(board.occupied_co[board.turn]))
    {
        optional<Piece>piece = board.piece_at(c);
        if (piece)
            evalu += PSQT[phase(board)==0][piece.value().piece_type][c]/10;
    }
    for (Square c:scan_reversed(board.occupied_co[!board.turn]))
    {
        optional<Piece> piece = board.piece_at(c);
        if (piece)
            evalu -= pesto_table[phase(board)==0][piece.value().piece_type][c]/10;
    }
    for (Square c:scan_reversed(board.occupied_co[!board.turn]))
    {
        optional<Piece> piece = board.piece_at(c);
        if (piece)
            evalu -= PSQT[phase(board)==0][piece.value().piece_type][c]/10;
    }
    evalu += evaluate_pawn_structure(board);
    evalu += space(board) - space(board.mirror());
    for (Square x:scan_reversed(board.occupied_co[board.turn]))
        evalu += is_trapped(board, x,!board.turn)*TrappedRook;
    for (Square x:scan_reversed(board.occupied_co[!board.turn]))
        evalu -= is_trapped(board, x,board.turn)*TrappedRook;
    return evalu;
}
