#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <sstream>
#include <string>
#include <vector>
#include "chess.h"
using namespace std;


int main()
{/*
    std::string input = "RNBQKBNR/PPPPPPPP/8/8/8/8/8/8 w KQkq";
    vector<string> out = TokenizeDefault(input, " ");
    std::string board_fen = out[0];
    regex v_castling_rights("[KQkq]");
    regex v_turn("[wb]");
    regex v_board("\\s*^(((?:[rnbqkpRNBQKP1-8]+\\/){7})[rnbqkpRNBQKP1-8]+)");
    // Create a match_results object to store the results
    std::smatch m;

    // Use regex_match to check if the input matches the pattern
    if (!std::regex_match(board_fen, m, v_board)) {
        std::cout << "Invalid board FEN" << std::endl;
        return -1;
    }
    string turn;
    if (out.size() == 1) {
        turn = 'w';
    }
    else if (out.size() >= 2) {
        turn = out[1];
    }
    // Use regex_match to check if the input matches the pattern
    if (!std::regex_match(turn, m, v_turn)) {
        std::cout << "Invalid turn" << std::endl;
        return -1;
    }
    string castling;
    if (out.size() == 1) {
        castling = "KQkq";
    }
    else if (out.size() >= 2) {
        castling = out[1];
    }
    // Use regex_match to check if the input matches the pattern
    if (!std::regex_match(turn, m, v_castling_rights)) {
        std::cout << "Invalid castling rights" << std::endl;
        //return -1;
    }*/

    Board board;
    board.clear_board();
    board.set_piece_at(0, KING, WHITE);
    board.set_piece_at(3, KING, BLACK);
    board.set_piece_at(8, PAWN, WHITE);
    board.set_piece_at(16, QUEEN, BLACK);
    cout << board.is_pinned(8) << endl;
    cout << board.str() << endl << endl;
    Board Board;
    Board.set_board_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    cout << Board.str() << endl;;
	return 0;
}
