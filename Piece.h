#ifndef PIECE_H
#define PIECE_H

#include <Arduino.h>

enum PieceType
{
  PAWN,
  ROOK,
  KNIGHT,
  BISHOP,
  QUEEN,
  KING
};

enum PieceColor
{
  WHITE,
  BLACK
};

class Piece
{
public:
  Piece(PieceType type, PieceColor color);
  PieceType getType() { return _type; }
  virtual const char *getTypeName() = 0;
  PieceColor getColor();

  virtual bool canMove(int fromRow, char fromCol, int toRow, char toCol) = 0;
  virtual ~Piece() {}
  void printInfo();

protected:
  PieceType _type;
  PieceColor _color;
};

#endif
