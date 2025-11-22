#include "Pawn.h"
#include <Arduino.h>

Pawn::Pawn(PieceColor color) : Piece(PAWN, color) {}

const char* Pawn::getTypeName() {
    return "Pawn";
}

bool Pawn::canMove(int fromRow, char fromCol, int toRow, char toCol) {
    int dir = (_color == WHITE) ? 1 : -1;
    int rowDiff = toRow - fromRow;
    int colDiff = toCol - fromCol;

    // Normal 1-square forward move
    if(rowDiff == dir && colDiff == 0) return true;

    // 2-square initial move
    if((_color == WHITE && fromRow == 2 || _color == BLACK && fromRow == 7) &&
       rowDiff == 2*dir && colDiff == 0) return true;

    // Diagonal capture
    if(rowDiff == dir && abs(colDiff) == 1) return true;

    return false;
}
