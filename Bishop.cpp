#include "Bishop.h"
#include <Arduino.h>

Bishop::Bishop(PieceColor color) : Piece(BISHOP, color) {}

const char* Bishop::getTypeName() {
    return "Bishop";
}

bool Bishop::canMove(int fromRow, char fromCol, int toRow, char toCol) {
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    return (rowDiff == colDiff) && rowDiff != 0;
}

