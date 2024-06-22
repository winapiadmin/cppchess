// chessexample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <vector>
#include "Board.h"
using namespace std;
void printBitboard(Bitboard bb) {
	for (int rank = 7; rank >= 0; --rank) {
		for (int file = 0; file < 8; ++file) {
			int square = rank * 8 + file;
			std::cout << (std::bitset<64>(bb).test(square)) << " ";
		}
		std::cout << std::endl;
	}
}
int perft(Board board, int depth){
    if (depth==0) return 1;
    int count=0;
    for (Move x:board.generate_legal_moves()){
        board.push(x);
        count += perft(board, depth-1);
        board.pop();
    };
    return count;
}
int main() {
    Board board;
    for (Move x:board.generate_legal_moves()){
        board.push(x);
        cout << x << ": " << perft(board,6) << endl;
        board.pop();
    }
	return 0;
}
