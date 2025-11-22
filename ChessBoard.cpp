#include "ChessBoard.h"
#include <Arduino.h>

enum PromotionType {
    PROMOTE_QUEEN,
    PROMOTE_ROOK,
    PROMOTE_BISHOP,
    PROMOTE_KNIGHT
};

ChessBoard::ChessBoard() {
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            board[r][c] = nullptr;
        }
    }
}

int ChessBoard::rowToIndex(int row) { return row - 1; }
int ChessBoard::colToIndex(char col) { return col - 'A'; }

Piece* ChessBoard::getPiece(int row, char col) {
    return board[rowToIndex(row)][colToIndex(col)];
}

void ChessBoard::placePiece(Piece* piece, int row, char col) {
    board[rowToIndex(row)][colToIndex(col)] = piece;
}

void ChessBoard::removePiece(int row, char col) {
    Piece* p = getPiece(row, col);
    if(p != nullptr){
        Serial.print("Removing piece: ");
        p->printInfo();
        board[rowToIndex(row)][colToIndex(col)] = nullptr;
    }
}

void ChessBoard::captureAndPlace(Piece* piece, int row, char col) {
    if(getPiece(row, col) != nullptr){
        removePiece(row, col);
    }
    placePiece(piece, row, col);
}

void ChessBoard::movePiece(int fromRow, char fromCol, int toRow, char toCol, PromotionType promoteChoice = PROMOTE_QUEEN) {
    Piece* piece = getPiece(fromRow, fromCol);
    if (!piece) {
        Serial.println("No piece to move!");
        return;
    }

    // Check if the piece can move
    if (!piece->canMove(fromRow, fromCol, toRow, toCol)) {
        Serial.println("Illegal move for this piece!");
        return;
    }

    // --- Special moves ---

    // King
    if (piece->getTypeName()[0] == 'K') {
        King* k = (King*)piece;

        // Castling
        if (abs(toCol - fromCol) == 2) {
            char rookCol = (toCol > fromCol) ? 'H' : 'A';
            Rook* r = (Rook*)getPiece(fromRow, rookCol);
            if (!r) {
                Serial.println("No rook to castle with!");
                return;
            }

            char newRookCol = (toCol > fromCol) ? 'F' : 'D';
            placePiece(r, fromRow, newRookCol);
            removePiece(fromRow, rookCol);

            r->hasMoved = true;
        }

        k->hasMoved = true;
    }

    // Rook
    if (piece->getTypeName()[0] == 'R') {
        Rook* r = (Rook*)piece;
        r->hasMoved = true;
    }

    // Pawn
    if (piece->getTypeName()[0] == 'P') {
        Pawn* pawn = (Pawn*)piece;
        int dir = (pawn->getColor() == WHITE) ? 1 : -1;

        // En passant
        if (abs(toCol - fromCol) == 1 && toRow - fromRow == dir && getPiece(toRow, toCol) == nullptr) {
            if (lastMove.piece && lastMove.piece->getTypeName()[0] == 'P') {
                if (abs(lastMove.toRow - lastMove.fromRow) == 2 &&
                    lastMove.toRow == fromRow &&
                    lastMove.toCol == toCol) {
                    removePiece(lastMove.toRow, lastMove.toCol);
                }
            }
        }

        // Update last move for en passant
        lastMove.piece = pawn;
        lastMove.fromRow = fromRow;
        lastMove.fromCol = fromCol;
        lastMove.toRow = toRow;
        lastMove.toCol = toCol;
    }

    // --- Move the piece ---
    captureAndPlace(piece, toRow, toCol);
    board[rowToIndex(fromRow)][colToIndex(fromCol)] = nullptr;

    // --- Pawn promotion ---
    if (piece->getTypeName()[0] == 'P') {
        int finalRow = (piece->getColor() == WHITE) ? 8 : 1;
        if (toRow == finalRow) {
            promotePawn(toRow, toCol, promoteChoice, piece->getColor());
        }
    }
}

void ChessBoard::printBoard() {
    for(int r=8; r>=1; r--){
        Serial.print(r); Serial.print(" ");
        for(char c='A'; c<='H'; c++){
            Piece* p = getPiece(r,c);
            if(p==nullptr) Serial.print(". ");
            else Serial.print(p->getTypeName()[0]), Serial.print(" ");
        }
        Serial.println();
    }
    Serial.println("  A B C D E F G H");
}

bool ChessBoard::isSquareAttacked(int row, char col, PieceColor attackerColor) {
    for(int r = 1; r <= 8; r++){
        for(char c = 'A'; c <= 'H'; c++){
            Piece* p = getPiece(r, c);
            if(p != nullptr && p->getColor() == attackerColor){
                if(p->canMove(r, c, row, col)){
                    return true;
                }
            }
        }
    }
    return false;
} 

void ChessBoard::promotePawn(int row, char col, PromotionType choice, PieceColor color) {
    Piece* pawn = getPiece(row, col);
    if(!pawn || pawn->getTypeName()[0] != 'P') {
        Serial.println("No pawn to promote on this square!");
        return;
    }

    removePiece(row, col);

    Piece* newPiece = nullptr;
    switch(choice) {
        case PROMOTE_QUEEN:
            newPiece = new Queen(color);
            break;
        case PROMOTE_ROOK:
            newPiece = new Rook(color);
            break;
        case PROMOTE_BISHOP:
            newPiece = new Bishop(color);
            break;
        case PROMOTE_KNIGHT:
            newPiece = new Knight(color);
            break;
    }

    if(newPiece) {
        placePiece(newPiece, row, col);
        Serial.print("Pawn promoted to ");
        Serial.print(newPiece->getTypeName());
        Serial.print(" at ");
        newPiece->printPosition();
        Serial.println();
    }
}