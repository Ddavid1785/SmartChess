#ifndef KNIGHT_H
#define KNIGHT_H

#include "Piece.h"

class Knight : public Piece {
  public:
    Knight(PieceColor color);

    const char* getTypeName() override;
    bool canMove(int fromRow, char fromCol, int toRow, char toCol) override;
};

#endif
