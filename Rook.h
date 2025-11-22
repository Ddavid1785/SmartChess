#ifndef ROOK_H
#define ROOK_H

#include "Piece.h"

class Rook : public Piece {
  public:
    Rook(PieceColor color);

    const char* getTypeName() override;
    bool canMove(int fromRow, char fromCol, int toRow, char toCol) override;

    bool hasMoved = false;
};

#endif
