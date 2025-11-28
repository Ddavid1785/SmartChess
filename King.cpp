#include "King.h"
#include "Rook.h"
#include <Arduino.h>

King::King(PieceColor color) : Piece(KING, color) {}

const char *King::getTypeName()
{
    return "King";
}

bool King::canMove(int fromRow, char fromCol, int toRow, char toCol)
{
    // Basic pattern check without board validation
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    // Normal king move (one square in any direction)
    if ((rowDiff <= 1 && colDiff <= 1) && (rowDiff + colDiff != 0))
    {
        return true;
    }

    // Castling (pattern check only - full validation needs board)
    if (!hasMoved && rowDiff == 0 && colDiff == 2)
    {
        return true;
    }

    return false;
}

bool King::canMove(int fromRow, char fromCol, int toRow, char toCol, ChessBoard *board)
{
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    if ((rowDiff <= 1 && colDiff <= 1) && (rowDiff + colDiff != 0))
    {
        if (board->isSquareAttacked(toRow, toCol, _color == WHITE ? BLACK : WHITE))
            return false; // cannot move into check
        return true;
    }

    if (!hasMoved && rowDiff == 0 && colDiff == 2)
    {
        if (board->isSquareAttacked(fromRow, fromCol, _color == WHITE ? BLACK : WHITE))
            return false;

        char rookCol = (toCol > fromCol) ? 'H' : 'A';
        Piece *rookPiece = board->getPiece(fromRow, rookCol);

        if (!rookPiece || rookPiece->getTypeName()[0] != 'R')
            return false;

        Rook *rook = (Rook *)rookPiece;

        if (rook->hasMoved)
            return false;

        char step = (toCol > fromCol) ? 1 : -1;
        for (char c = fromCol + step; c != rookCol; c += step)
        {
            if (board->getPiece(fromRow, c) != nullptr)
                return false; // blocked
            if (board->isSquareAttacked(fromRow, c, _color == WHITE ? BLACK : WHITE))
                return false;
        }

        return true;
    }

    return false;
}
