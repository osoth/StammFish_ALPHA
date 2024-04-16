
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#include "engine.h"
    ChessBoard::ChessBoard() {
        pieces[0] = 0b0000000000000000000000000000000000000000000000001111111100000000; // white pawns
        pieces[1] = 0b0000000000000000000000000000000000000000000000000000000010000001; // white rooks
        pieces[2] = 0b0000000000000000000000000000000000000000000000000000000001000010; // white knights
        pieces[3] = 0b0000000000000000000000000000000000000000000000000000000000100100; // white bishops
        pieces[4] = 0b0000000000000000000000000000000000000000000000000000000000010000; // white queens
        pieces[5] = 0b0000000000000000000000000000000000000000000000000000000000001000; // white king

        pieces[6] = 0b0000000011111111000000000000000000000000000000000000000000000000; // black pawns
        pieces[7] = 0b1000000100000000000000000000000000000000000000000000000000000000; // black rooks
        pieces[8] = 0b0100001000000000000000000000000000000000000000000000000000000000; // black knights
        pieces[9] = 0b0010010000000000000000000000000000000000000000000000000000000000; // black bishops
        pieces[10] = 0b0001000000000000000000000000000000000000000000000000000000000000;// black queens
        pieces[11] = 0b0000100000000000000000000000000000000000000000000000000000000000;// black king
        occupancies[0] = pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5];
        occupancies[1] = pieces[6] | pieces[7] | pieces[8] | pieces[9] | pieces[10] | pieces[11];
        getAllVision(occupancies);
    }

    std::string ChessBoard::getGameState(){
        if (halfMoveClock >= 50) {
            return "draw";
        }
        if (getValidMoves().empty()) {
            if (isCheck()) {
                return "checkmate";
            } else {
                return "draw";
            }
        }
        return "none";
    }
    void ChessBoard::printOccupancy(){
        for (int i = 63; i >= 0; i--) {
            if (i % 8 == 7) {
                std::cout << std::endl;
            }
            if (occupancies[0] & (1ULL << i)) {
                std::cout << "1 ";
            } else {
                std::cout << "0 ";
            }
        }
        std::cout << std::endl;
        for (int i = 63; i >= 0; i--) {
            if (i % 8 == 7) {
                std::cout << std::endl;
            }
            if (occupancies[1] & (1ULL << i)) {
                std::cout << "1 ";
            } else {
                std::cout << "0 ";
            }
        }
    }
    void ChessBoard::getAllVision(uint64_t occu[]){
        std::vector<int> indices = getSetBitIndices(occu[0]);
        std::vector<uint64_t> pieceVisions;
        for (int index : indices) {
            pieceVisions.push_back(getVision(index));
        }
        visions[0] = 0;
        for (uint64_t pieceVision : pieceVisions) {
            visions[0] |= pieceVision;
        }
        white_Visions = pieceVisions;
        indices = getSetBitIndices(occu[1]);
        pieceVisions.clear();
        for (int index : indices) {
            pieceVisions.push_back(getVision(index));
        }
        visions[1] = 0;
        for (uint64_t pieceVision : pieceVisions) {
            visions[1] |= pieceVision;
        }
    }
    void ChessBoard::printVision(){
     // print the bits 8 at a time
     for (int i = 63; i >= 0; i--) {
         if (i % 8 == 7) {
             std::cout << std::endl;
         }
         if (visions[0] & (1ULL << i)) {
             std::cout << "1 ";
         } else {
             std::cout << "0 ";
         }
     }
        std::cout << std::endl;
     for (int i = 63; i >= 0; i--) {
            if (i % 8 == 7) {
                std::cout << std::endl;
            }
            if (visions[1] & (1ULL << i)) {
                std::cout << "1 ";
            } else {
                std::cout << "0 ";
            }
     }
    }
    void ChessBoard::printBoard() {
        // "PRNBQK" for white pieces, "prnbqk" for black pieces
        // black pieces are in the upper half of the board
        for (int i = 7; i >=0 ; i--) {
            for (int j = 7; j >= 0; j--) {
                char piece = ' ';
                for (int k = 11; k >= 0; k--) {
                    if (pieces[k] & (1ULL << (i * 8 + j))) {
                        piece = "PRNBQKprnbqk"[k];
                        break;
                    }
                    piece = '.';
                }
                std::cout << piece << " ";
            }
            std::cout << std::endl;
        }
    }
    void ChessBoard::loadFEN(const std::string& fen) {
        std::istringstream iss(fen);
        std::string field;
        int rank = 7, file = 0;

        // Reset the board
        for (unsigned long & piece : pieces) {
            piece = 0;
        }

        // Process piece placement
        std::getline(iss, field, ' ');
        for (char c : field) {
            if (c == '/') {
                rank--;
                file = 0;
            } else if (isdigit(c)) {
                file += c - '0';
            } else {
                std::string::size_type piece_pos = std::string("PRNBQKprnbqk").find(c);
                int piece = static_cast<int>(piece_pos);
                pieces[piece] |= 1ULL << ((rank) * 8 + (7 - file));
                file++;
            }
        }

        // Process active color
        std::getline(iss, field, ' ');
        turn = (field == "w") ? 0 : 1;

        // Process castling availability
        std::getline(iss, field, ' ');
        if (field != "-") {
            castling[0] = 0;
            castling[1] = 0;
            castling[2] = 0;
            castling[3] = 0;
            for (char c : field) {
                int index = (c == 'K') ? 0 : (c == 'Q') ? 1 : (c == 'k') ? 2 : 3;
                castling[index] = 1;
            }
        } else {
            for (int & i : castling) {
                i = 0;
            }
        }

        // Process en passant target square
        std::getline(iss, field, ' ');
        if (field != "-") {
            int file2 = field[0] - 'a';
            int rank2 = field[1] - '1';
            enPassant = 1ULL << (rank2 * 8 + file2);
        }else {
            enPassant = 0;
        }

        // Process half-move clock
        std::getline(iss, field, ' ');
        halfMoveClock = std::stoi(field);

        // Process full-move number
        std::getline(iss, field, ' ');
        fullMoveNumber = std::stoi(field);
        occupancies[0] = pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5];
        occupancies[1] = pieces[6] | pieces[7] | pieces[8] | pieces[9] | pieces[10] | pieces[11];
        getAllVision(occupancies);
    }
    std::string ChessBoard::getFEN(){
        std::string fen;
        for (int i = 7; i >= 0; i--) {
            int empty = 0;
            for (int j = 7; j >= 0; j--) {
                char piece = ' ';
                for (int k = 11; k >= 0; k--) {
                    if (pieces[k] & (1ULL << (i * 8 + j))) {
                        piece = "PRNBQKprnbqk"[k];
                        break;
                    }
                }
                if (piece == ' ') {
                    empty++;
                } else {
                    if (empty > 0) {
                        fen += std::to_string(empty);
                        empty = 0;
                    }
                    fen += piece;
                }
            }
            if (empty > 0) {
                fen += std::to_string(empty);
            }
            if (i > 0) {
                fen += '/';
            }
        }
        fen += (turn == 0) ? " w " : " b ";
        // Add castling availability
        if (castling[0] == 0 && castling[1] == 0 && castling[2] == 0 && castling[3] == 0) {
            fen += "-";
        } else {
            for (int i = 0; i < 4; i++) {
                if (castling[i] == 1) {
                    fen += "KQkq"[i];
                }
            }
            fen += " ";
        }
        // Add en passant target square
        if (enPassant == 0) {
            fen += "- ";
        } else {
            fen += numberToSquare(__builtin_ctzll(enPassant)) + " ";
        }
        // Add half-move clock
        fen += std::to_string(halfMoveClock) + " ";
        // Add full-move number
        fen += std::to_string(fullMoveNumber);
        return fen;
    }
    std::string ChessBoard::numberToSquare(int num){
        num = 63 - num;
        int line = num % 8;
        int row = num / 8;
        row = (8 - row);
        return std::string(1, static_cast<char>('a' + line)) + std::to_string(row);
    }
    int ChessBoard::evaluateBoard() {
        int pieceValues[12] = {1, 3, 3, 5, 9, 0, 1, 3, 3, 5, 9, 0};  // Values for white pawns, knights, bishops, rooks, queens, king, black pawns, knights, bishops, rooks, queens, king
        int score = 0;

        for (int i = 0; i < 12; i++) {
            score += __builtin_popcountll(pieces[i]) * pieceValues[i];
        }

        return score;
    }
    int ChessBoard::minimax(ChessBoard board, int depth, int alpha, int beta, bool maximizingPlayer) {
        if (depth == 0 || getGameState() != "none") {
            return board.evaluateBoard();  // You need to implement this function
        }

        if (maximizingPlayer) {
            int maxEval = -9999;
            std::vector<std::string> validMoves = getValidMoves();
            for (const auto & move : validMoves) {
                ChessBoard tempBoard = board;
                tempBoard.MakeMove(move, false);
                int eval = tempBoard.minimax(tempBoard, depth - 1, alpha, beta, false);
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) {
                    break;
                }
            }
            return maxEval;
        } else {
            int minEval = 9999;
            std::vector<std::string> validMoves = getValidMoves();
            for (const auto & move : validMoves) {
                ChessBoard tempBoard = *this;
                tempBoard.MakeMove(move, false);
                int eval = tempBoard.minimax(tempBoard, depth - 1, alpha, beta, true);
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) {
                    break;
                }
            }
            return minEval;
        }
    }

    std::string ChessBoard::bestMove(int depth) {
        int maxEval = -9999;
        std::string bestMove;
        std::vector<std::string> validMoves = getValidMoves();
        for (const auto & move : validMoves) {
            ChessBoard tempBoard = *this;
            tempBoard.MakeMove(move, false);
            int eval = tempBoard.minimax(tempBoard, depth - 1, -9999, 9999, false);
            if (eval > maxEval) {
                maxEval = eval;
                bestMove = move;
            }
        }
        return bestMove;
        std::cout << maxEval << std::endl;
    }

    int ChessBoard::squareToNumber(const std::string& square){
        int line = square[0] - 'a';
        int temp_row = square[1] - '0';
        int row = (8-temp_row) * 8;
        return 63 - (line + row);
    }


    void ChessBoard::MakeMove(const std::string& move, bool ValidateMove) {
        // if move not in valid moves return
        if (ValidateMove) {
            std::vector<std::string> validMoves = getValidMoves();
            auto it = std::find(validMoves.begin(), validMoves.end(), move) == validMoves.end();
            if (it) {
                std::cerr << "Invalid move" << std::endl;
                return;
            }
        }
        enPassant = 0;
        int fromIndex;
        int toIndex;
        // split the move into parts
        if (move != "O-O" && move != "O-O-O") {
            std::string from = move.substr(0, 2);
            std::string to = move.substr(2, 2);
            // check which piece is on that square
            fromIndex = squareToNumber(from);
            toIndex = squareToNumber(to);
        }else if (move == "O-O") {
            fromIndex = (turn == 0) ? 3 : 59;
            toIndex = (turn == 0) ? 1 : 57;
        }else{
            fromIndex = (turn == 0) ? 3 : 59;
            toIndex = (turn == 0) ? 5 : 61;
        }
        // find bitboard that has the piece
        if (turn == 1){
            fullMoveNumber++;
        }
        if (move == "O-O"){
            if (turn == 0){
                pieces[5] &= ~(1ULL << 3);
                pieces[5] |= (1ULL << 1);
                pieces[1] &= ~(1ULL << 0);
                pieces[1] |= (1ULL << 2);
                castling[0] = 0;
                castling[1] = 0;
            }else{
                pieces[11] &= ~(1ULL << 59);
                pieces[11] |= (1ULL << 57);
                pieces[7] &= ~(1ULL << 56);
                pieces[7] |= (1ULL << 58);
                castling[2] = 0;
                castling[3] = 0;
            }
            halfMoveClock++;
            turn = (turn + 1) % 2;
            occupancies[0] = pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5];
            occupancies[1] = pieces[6] | pieces[7] | pieces[8] | pieces[9] | pieces[10] | pieces[11];
            getAllVision(occupancies);
            return;
        }else if (move == "O-O-O"){
            if (turn == 0){
                pieces[5] &= ~(1ULL << 3);
                pieces[5] |= (1ULL << 5);
                pieces[1] &= ~(1ULL << 7);
                pieces[1] |= (1ULL << 4);
                castling[0] = 0;
                castling[1] = 0;
            }else{
                pieces[11] &= ~(1ULL << 59);
                pieces[11] |= (1ULL << 61);
                pieces[7] &= ~(1ULL << 63);
                pieces[7] |= (1ULL << 60);
                castling[2] = 0;
                castling[3] = 0;
            }
            halfMoveClock++;
            turn = (turn + 1) % 2;
            occupancies[0] = pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5];
            occupancies[1] = pieces[6] | pieces[7] | pieces[8] | pieces[9] | pieces[10] | pieces[11];
            getAllVision(occupancies);
            return;
        }else if (move.length() == 5){
            // promote the pawn
            if (turn == 0) {
                pieces[0] &= ~(1ULL << fromIndex);
                if (move[4] == 'q') {
                    pieces[4] |= (1ULL << toIndex);
                } else if (move[4] == 'r') {
                    pieces[1] |= (1ULL << toIndex);
                } else if (move[4] == 'b') {
                    pieces[3] |= (1ULL << toIndex);
                } else {
                    pieces[2] |= (1ULL << toIndex);
                }

            } else {
                pieces[6] &= ~(1ULL << fromIndex);
                if (move[4] == 'q') {
                    pieces[10] |= (1ULL << toIndex);
                } else if (move[4] == 'r') {
                    pieces[7] |= (1ULL << toIndex);
                } else if (move[4] == 'b') {
                    pieces[9] |= (1ULL << toIndex);
                } else {
                    pieces[8] |= (1ULL << toIndex);
                }
            }
            halfMoveClock++;
            turn = (turn + 1) % 2;
            occupancies[0] = pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5];
            occupancies[1] = pieces[6] | pieces[7] | pieces[8] | pieces[9] | pieces[10] | pieces[11];
            getAllVision(occupancies);
        }else{
            halfMoveClock++;
            for (unsigned long &piece: pieces) {
                if (piece & (1ULL << fromIndex)) {
                    // delete the piece from the original square and add it to the new square if there is a piece there it will be deleted from the other bitboard
                    if (piece == pieces[5] || piece == pieces[11]) {
                        castling[0 + 2 * turn] = 0;
                        castling[1 + 2 * turn] = 0;
                    }
                    if (piece == pieces[0]) {
                        halfMoveClock = 0;
                        // check for absolute value and if there is a piece from pieces[6] on the square to the left or right
                        if (abs(fromIndex - toIndex) == 16 &&
                                ((pieces[6] & (1ULL << (toIndex - 1))) || (pieces[6] & (1ULL << (toIndex + 1))))) {
                            enPassant = 1ULL << (toIndex - 8);
                        }
                    }
                    if (piece == pieces[6]) {
                        halfMoveClock = 0;
                        if (abs(fromIndex - toIndex) == 16 && ((pieces[0] & (1ULL << (toIndex - 1))) || (pieces[0] & (1ULL << (toIndex + 1))))) {
                            enPassant = 1ULL << (toIndex + 8);
                        }
                    }
                    piece &= ~(1ULL << fromIndex);
                    piece |= (1ULL << toIndex);

                    continue;
                }
                if (piece & (1ULL << toIndex)) {
                    piece &= ~(1ULL << toIndex);
                    halfMoveClock = 0;
                }
            }
            turn = (turn + 1) % 2;
            occupancies[0] = pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5];
            occupancies[1] = pieces[6] | pieces[7] | pieces[8] | pieces[9] | pieces[10] | pieces[11];
            getAllVision(occupancies);
        }
    }

std::vector<std::string> ChessBoard::getValidMoves(){
    std::vector<std::tuple<int, int>> moves;
    std::vector<std::string> validMoves;
    std::vector<std::string> promotionMoves;
    if (turn == 0){
        std::vector<int> indices = getSetBitIndices(occupancies[0]);
        for (int index : indices) {
            uint64_t vision = getVision(index);
            std::vector<int> visionIndices = getSetBitIndices(vision);
            for (int visionIndex : visionIndices) {
                if (!(occupancies[0] & (1ULL << visionIndex))) {
                    if (pieces[0] & (1ULL << index) && visionIndex / 8 == 7) { // Check if it's a white pawn moving to the last rank
                        std::string move = numberToSquare(index) + numberToSquare(visionIndex);
                        promotionMoves.push_back(move + "q");
                        promotionMoves.push_back(move + "r");
                        promotionMoves.push_back(move + "b");
                        promotionMoves.push_back(move + "n");
                    } else {
                        moves.emplace_back(index, visionIndex);
                    }
                }
            }
        }
        // check if the king can castle
        if (castling[0] == 1 && !(visions[1] & pieces[5] >> 1) && !(pieces[5] >> 1 & occupancies[0]) && !(pieces[5] >> 2 & occupancies[0])){
            validMoves.emplace_back("O-O");
        }
        if (castling[1] == 1 && !(visions[1] & pieces[5] << 1) && !(pieces[5] << 1 & occupancies[0]) && !(pieces[5] << 2 & occupancies[0]) && !(pieces[5] << 3 & occupancies[0])){
            validMoves.emplace_back("O-O-O");
        }
    } else {
        std::vector<int> indices = getSetBitIndices(occupancies[1]);
        for (int index : indices) {
            uint64_t vision = getVision(index);
            std::vector<int> visionIndices = getSetBitIndices(vision);
            for (int visionIndex : visionIndices) {
                if (!(occupancies[1] & (1ULL << visionIndex))) {
                    if (pieces[6] & (1ULL << index) && visionIndex / 8 == 0) { // Check if it's a black pawn moving to the last rank
                        std::string move = numberToSquare(index) + numberToSquare(visionIndex);
                        promotionMoves.push_back(move + "q");
                        promotionMoves.push_back(move + "r");
                        promotionMoves.push_back(move + "b");
                        promotionMoves.push_back(move + "n");
                    } else {
                        moves.emplace_back(index, visionIndex);
                    }
                }
            }
        }
        if (castling[2] == 1 && !(visions[0] & pieces[11] >> 1) && !(pieces[11] >> 1 & occupancies[1]) && !(pieces[11] >> 2 & occupancies[1])){
            validMoves.emplace_back("O-O");
        }
        if (castling[3] == 1 && !(visions[0] & pieces[11] << 1) && !(pieces[11] << 1 & occupancies[1]) && !(pieces[11] << 2 & occupancies[1]) && !(pieces[11] << 3 & occupancies[1])){
            validMoves.emplace_back("O-O-O");
        }
    }

    for (auto move : moves) {
        validMoves.push_back(numberToSquare(std::get<0>(move)) + numberToSquare(std::get<1>(move)));
    }

    // Add promotion moves
    validMoves.insert(validMoves.end(), promotionMoves.begin(), promotionMoves.end());

    std::vector<std::string> trulyValidMoves;
    for (const std::string& move : validMoves) {
        ChessBoard tempBoard = *this;
        tempBoard.turn = (turn + 1) % 2;
        tempBoard.MakeMove(move, false);
        if (!tempBoard.isCheck()) {
            trulyValidMoves.push_back(move);
        }
    }

    if (trulyValidMoves.empty()){
        std::cerr << "Checkmate" + (turn == 0 ? std::string("black wins") : std::string("white wins")) << std::endl;
    }

    return trulyValidMoves;
}

    uint64_t ChessBoard::getVision(int num) {
        for (int i = 0; i < 12; i++) {
            if (pieces[i] & (1ULL << num)) {
                if (i == 0 || i == 6) {
                    return getPawnVision(num, i);
                } else if (i == 1 || i == 7) {
                    return getRookVision(num, i);
                } else if (i == 2 || i == 8) {
                    return getKnightVision(num, i);
                } else if (i == 3 || i == 9) {
                    return getBishopVision(num, i);
                } else if (i == 4 || i == 10) {
                    return getQueenVision(num, i);
                } else if (i == 5 || i == 11) {
                    return getKingVision(num, i);
                }
            }
        }
        return 0;
    }
    std::vector<int> ChessBoard::getSetBitIndices(uint64_t num) {
        std::vector<int> indices;
        for (int i = 0; i < 64; ++i) {
            if (num & (1ULL << i)) {
                indices.push_back(i);
            }
        }
        return indices;
    }

    /* get vision for all pieces
     *
     * getPawnVision
    */

    uint64_t ChessBoard::getPawnVision(int num, int i) {
        uint64_t vision = 0;
        if (i == 0) {
            if (num / 8 < 7 && (!((1ULL << (num + 8)) & occupancies[1]) && !((1ULL << (num + 8)) & occupancies[0]))) {
                vision |= 1ULL << (num + 8);
            }
            if (num < 16 && (!((1ULL << (num + 16)) & occupancies[1]) && !((1ULL << (num + 16)) & occupancies[0])) && (!((1ULL << (num + 8)) & occupancies[1]) && !((1ULL << (num + 8)) & occupancies[0]))) {
                vision |= 1ULL << (num + 16);
            }
            if ((num % 8 < 7 && num / 8 < 7 && (1ULL << (num + 9)) & occupancies[1]) || (num % 8 < 7 && num / 8 < 7 && (1ULL << (num + 9)) & enPassant)) {
                vision |= 1ULL << (num + 9);
            }
            if ((num % 8 > 0 && num / 8 < 7 && (1ULL << (num + 7)) & occupancies[1]) || (num % 8 > 0 && num / 8 < 7 && (1ULL << (num + 7)) & enPassant)){
                vision |= 1ULL << (num + 7);
            }
        } else {
            if (num / 8 > 0 && (!((1ULL << (num - 8)) & occupancies[0]) && !((1ULL << (num - 8)) & occupancies[1]))) {
                vision |= 1ULL << (num - 8);
            }
            if (num > 47 && (!((1ULL << (num - 16)) & occupancies[0]) && !((1ULL << (num - 16)) & occupancies[1])) && (!((1ULL << (num - 8)) & occupancies[0]) && !((1ULL << (num - 8)) & occupancies[1]))) {
                vision |= 1ULL << (num - 16);
            }
            if ((num % 8 < 7 && num / 8 > 0 && (1ULL << (num - 7)) & occupancies[0]) || (num % 8 < 7 && num / 8 > 0 && (1ULL << (num - 7)) & enPassant)) {
                vision |= 1ULL << (num - 7);
            }
            if ((num % 8 > 0 && num / 8 > 0 && (1ULL << (num - 9)) & occupancies[0]) || (num % 8 > 0 && num / 8 > 0 && (1ULL << (num - 9)) & enPassant)){
                vision |= 1ULL << (num - 9);
            }
        }
        return vision;
    }
    uint64_t ChessBoard::getRookVision(int num, int i) {
        uint64_t rook_attacks = attacks.Rook(occupancies[0] | occupancies[1], num);
        rook_attacks &= ~occupancies[i == 1 ? 0 : 1];
        return rook_attacks;
    }
    uint64_t ChessBoard::getBishopVision(int num, int i) {
        uint64_t bishop_attacks = attacks.Bishop(occupancies[0] | occupancies[1], num);
        bishop_attacks &= ~occupancies[i == 3 ? 0 : 1];
        return bishop_attacks;
    }
    uint64_t ChessBoard::getQueenVision(int num, int i) {
        uint64_t queen_attacks = attacks.Queen(occupancies[0] | occupancies[1], num);
        queen_attacks &= ~occupancies[i == 4 ? 0 : 1];
        return queen_attacks;
    }
    uint64_t ChessBoard::getKnightVision(int num, int i) {
        uint64_t vision = knightAttacks[num];
        vision = vision ^ occupancies[i == 2 ? 0 : 1];
        return vision;
    }
    uint64_t ChessBoard::getKingVision(int num, int i) {
        uint64_t vision = 0;
        uint64_t startPos = 1ULL << num;
        if (num % 8 < 7) {
            startPos = startPos << 1;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])) {
                vision |= startPos;
            }
        }
        startPos = 1ULL << num;
        if (num % 8 > 0){
            startPos = startPos >> 1;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])){
                vision |= startPos;
            }
        }
        startPos = 1ULL << num;
        if (num / 8 < 7){
            startPos = startPos << 8;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])){
                vision |= startPos;
            }
        }
        startPos = 1ULL << num;
        if (num / 8 > 0){
            startPos = startPos >> 8;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])){
                vision |= startPos;
            }
        }
        startPos = 1ULL << num;
        if (num % 8 < 7 && num / 8 < 7) {
            startPos = startPos << 9;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])) {
                vision |= startPos;
            }
        }
        startPos = 1ULL << num;
        if (num % 8 > 0 && num / 8 < 7){
            startPos = startPos << 7;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])){
                vision |= startPos;
            }
        }
        startPos = 1ULL << num;
        if (num % 8 < 7 && num / 8 > 0){
            startPos = startPos >> 7;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])){
                vision |= startPos;
            }
        }
        startPos = 1ULL << num;
        if (num % 8 > 0 && num / 8 > 0){
            startPos = startPos >> 9;
            if (!(startPos & occupancies[i == 5 ? 0 : 1])){
                vision |= startPos;
            }
        }
        return vision;
    }
    bool ChessBoard::isCheck() {
        return (turn == 0) ? (visions[1] & pieces[5]) : (visions[0] & pieces[11]);
    }


namespace py = pybind11;

PYBIND11_MODULE(chess_engine, m) {
    py::class_<ChessBoard>(m, "ChessBoard")
    .def(py::init<>())
    .def("bestMove", &ChessBoard::bestMove)
    .def("printOccupancy", &ChessBoard::printOccupancy)
    .def("getAllVision", &ChessBoard::getAllVision)
    .def("printVision", &ChessBoard::printVision)
    .def("printBoard", &ChessBoard::printBoard)
    .def("numberToSquare", &ChessBoard::numberToSquare)
    .def("squareToNumber", &ChessBoard::squareToNumber)
    .def("getValidMoves", &ChessBoard::getValidMoves)
    .def("MakeMove", &ChessBoard::MakeMove)
    .def("getVision", &ChessBoard::getVision)
    .def("getSetBitIndices", &ChessBoard::getSetBitIndices)
    .def("getPawnVision", &ChessBoard::getPawnVision)
    .def("getRookVision", &ChessBoard::getRookVision)
    .def("getBishopVision", &ChessBoard::getBishopVision)
    .def("getKnightVision", &ChessBoard::getKnightVision)
    .def("getQueenVision", &ChessBoard::getQueenVision)
    .def("getKingVision", &ChessBoard::getKingVision)
    .def("isCheck", &ChessBoard::isCheck)
    .def("loadFEN", &ChessBoard::loadFEN)
    .def("getFEN", &ChessBoard::getFEN)
    .def("getGameState", &ChessBoard::getGameState);
}

