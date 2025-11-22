#ifndef QUEEN_H
#define QUEEN_H

#include "Piece.h"

class Queen : public Piece {
  public:
    Queen(PieceColor color);

    const char* getTypeName() override;
    bool canMove(int fromRow, char fromCol, int toRow, char toCol) override;
};

#endif
