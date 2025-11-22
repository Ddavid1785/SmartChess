#ifndef PAWN_H
#define PAWN_H

#include "Piece.h"

class Pawn : public Piece {
  public:
    Pawn(PieceColor color);

    const char* getTypeName() override;
    bool canMove(int fromRow, char fromCol, int toRow, char toCol) override;
};

#endif
