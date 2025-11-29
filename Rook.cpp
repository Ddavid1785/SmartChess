#include "Rook.h"
#include <Arduino.h>

Rook::Rook(PieceColor color) : Piece(ROOK, color) {}

const char *Rook::getTypeName()
{
    return "Rook";
}

bool Rook::canMove(int fromRow, char fromCol, int toRow, char toCol)
{
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    return (rowDiff == 0 && colDiff != 0) || (rowDiff != 0 && colDiff == 0);
}
