#ifndef KING_H
#define KING_H

#include "Piece.h"

class ChessBoard; // forward declaration

class King : public Piece {
  public:
    King(PieceColor color);

    const char* getTypeName() override;

    bool canMove(int fromRow, char fromCol, int toRow, char toCol) override;

    bool canMove(int fromRow, char fromCol, int toRow, char toCol, ChessBoard* board);

    bool hasMoved = false;
};

#endif
