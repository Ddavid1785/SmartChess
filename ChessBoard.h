#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <Arduino.h>
#include "Piece.h"

class ChessBoard {

struct LastMove {
    Piece* piece;
    int fromRow;
    char fromCol;
    int toRow;
    char toCol;
};

LastMove lastMove;

  public:
    ChessBoard();

    void placePiece(Piece* piece, int row, char col);
    void removePiece(int row, char col);
    void movePiece(int fromRow, char fromCol, int toRow, char toCol);
    void captureAndPlace(Piece* piece, int row, char col);
    bool isSquareAttacked(int row, char col, PieceColor attackerColor);

    Piece* getPiece(int row, char col);
    void printBoard();

  private:
    Piece* board[8][8];

    int rowToIndex(int row);
    int colToIndex(char col);
};

#endif
