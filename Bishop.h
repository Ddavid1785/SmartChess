#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.h"

class Bishop : public Piece {
  public:
    Bishop(PieceColor color);

    const char* getTypeName() override;
    bool canMove(int fromRow, char fromCol, int toRow, char toCol) override;
};

#endif
