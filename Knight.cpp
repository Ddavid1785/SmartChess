#include "Knight.h"
#include <Arduino.h>

Knight::Knight(PieceColor color) : Piece(KNIGHT, color) {}

const char* Knight::getTypeName() {
    return "Knight";
}

bool Knight::canMove(int fromRow, char fromCol, int toRow, char toCol) {
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    if((rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2))
        return true;

    return false;
}
