// chessexample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "Board.h"
//#include "eval.h"
#include <chrono>
using namespace std;
void printBitboard(Bitboard bb)
{
    for (int rank = 7; rank >= 0; --rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            int square = rank * 8 + file;
            std::cout << (std::bitset<64>(bb).test(square)) << " ";
        }
        std::cout << std::endl;
    }
}
int perft(Board &board, int depth)
{
    if (depth==0) return 1;
    int count=0;
    for (Move x:board.generate_legal_moves())
    {
        board.push(x);
        count += perft(board, depth-1);
        board.pop();
    };
    return count;
}
int main()
{
    Board board;
    for (int i=1; i < 7; i++)
    {
        auto ts = std::chrono::steady_clock::now();
        unsigned long long nodes=perft(board,i);
        auto te = std::chrono::steady_clock::now();
        unsigned long long total = std::chrono::duration_cast<std::chrono::nanoseconds>(te - ts).count();
        cout <<  "Depth " << i << " Time: "<<total/1e9 << "s " << nodes << " " <<setprecision(10)<< ((long double)nodes/total*1000) << " MNPS"<< endl;
    }
    return 0;
}
