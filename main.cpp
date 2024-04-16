#include <iostream>
#include <vector>
#include "engine.h"

int main() {
    // listen for input
    std::string input;
    ChessBoard board;
    while (true) {
        std::getline(std::cin, input);
        if (input == "uci") {
            std::cout << "id name Chess Engine\n";
            std::cout << "id author Keshav\n";
            std::cout << "uciok\n";
        } else if (input == "isready") {
            std::cout << "readyok\n";
        }else if (input == "validMoves"){
            std::vector<std::string> moves = board.getValidMoves();
            for (const auto & move : moves) {
                std::cout << move << " ";
            }
            std::cout << "\n";
        }else if (input == "print") {
            board.printBoard();
        }else if (input.substr(0, 4) == "move"){
                board.MakeMove(input.substr(5));
                std::cout << "made Move\n" << " ";
        }else if (input == "reset") {
            board.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }else if (input.substr(0, 8) == "position") {
            if (input.substr(9, 8) == "startpos") {
                board.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            } else {
                std::string fen = input.substr(9);
                board.loadFEN(fen);
            }
        } else if (input.substr(0, 2) == "go") {
            std::string bestMove = board.bestMove(4);
            std::cout << bestMove << "\n";
        } else if (input == "quit") {
            break;
        }
    }
}