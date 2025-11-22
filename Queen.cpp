#include "Queen.h"
#include <Arduino.h>

Queen::Queen(PieceColor color) : Piece(QUEEN, color) {}

const char* Queen::getTypeName() {
    return "Queen";
}

bool Queen::canMove(int fromRow, char fromCol, int toRow, char toCol) {
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    if((rowDiff == 0 && colDiff != 0) || (rowDiff != 0 && colDiff == 0))
        return true;
    if(rowDiff == colDiff && rowDiff != 0)
        return true;

    return false;
}
