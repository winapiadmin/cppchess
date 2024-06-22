#include "movegen.h"

    Bitboard _sliding_attacks(Square square, Bitboard occupied, const std::vector<int> &deltas)
    {
        Bitboard attacks = BB_EMPTY;

        for (int delta : deltas)
        {
            Square sq = square;

            while (true)
            {
                sq += delta;
                if (!(0 <= sq && sq < 64) || square_distance(sq, sq - delta) > 2)
                {
                    break;
                }

                attacks |= BB_SQUARES[sq];

                if (occupied & BB_SQUARES[sq])
                {
                    break;
                }
            }
        }

        return attacks;
    }

    Bitboard _step_attacks(Square square, const std::vector<int> &deltas)
    {
        return _sliding_attacks(square, BB_ALL, deltas);
    }

    Bitboard _edges(Square square)
    {
        return (((BB_RANK_1 | BB_RANK_8) & ~BB_RANKS[square_rank(square)]) |
                ((BB_FILE_A | BB_FILE_H) & ~BB_FILES[square_file(square)]));
    }

    std::vector<Bitboard> _carry_rippler(Bitboard mask)
    {
        // Carry-Rippler trick to iterate subsets of mask.
        std::vector<Bitboard> iter;
        Bitboard subset = BB_EMPTY;
        while (true)
        {
            iter.push_back(subset);
            subset = (subset - mask) & mask;
            if (!subset)
            {
                break;
            }
        }
        return iter;
    }

    std::tuple<std::vector<Bitboard>, std::vector<std::unordered_map<Bitboard, Bitboard>>> _attack_table(const std::vector<int> &deltas)
    {
        std::vector<Bitboard> mask_table;
        std::vector<std::unordered_map<Bitboard, Bitboard>> attack_table;

        for (Square square : SQUARES)
        {
            std::unordered_map<Bitboard, Bitboard> attacks;

            Bitboard mask = _sliding_attacks(square, 0, deltas) & ~_edges(square);
            for (Bitboard subset : _carry_rippler(mask))
            {
                attacks[subset] = _sliding_attacks(square, subset, deltas);
            }

            attack_table.push_back(attacks);
            mask_table.push_back(mask);
        }

        return {mask_table, attack_table};
    }

    std::vector<std::vector<Bitboard>> _rays()
    {
        std::vector<std::vector<Bitboard>> rays;
        for (int a = 0; a < 64; ++a)
        {
            Bitboard bb_a = BB_SQUARES[a];
            std::vector<Bitboard> rays_row;
            for (int b = 0; b < 64; ++b)
            {
                Bitboard bb_b = BB_SQUARES[b];
                if (BB_DIAG_ATTACKS[a].at(0) & bb_b)
                {
                    rays_row.push_back((BB_DIAG_ATTACKS[a].at(0) & BB_DIAG_ATTACKS[b].at(0)) | bb_a | bb_b);
                }
                else if (BB_RANK_ATTACKS[a].at(0) & bb_b)
                {
                    rays_row.push_back(BB_RANK_ATTACKS[a].at(0) | bb_a);
                }
                else if (BB_FILE_ATTACKS[a].at(0) & bb_b)
                {
                    rays_row.push_back(BB_FILE_ATTACKS[a].at(0) | bb_a);
                }
                else
                {
                    rays_row.push_back(BB_EMPTY);
                }
            }
            rays.push_back(rays_row);
        }
        return rays;
    }
