#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <Arduino.h>
#include "Piece.h"

enum GameState
{
  GAME_ACTIVE,
  GAME_CHECKMATE_WHITE,
  GAME_CHECKMATE_BLACK,
  GAME_STALEMATE,
  GAME_DRAW
};

enum PromotionType
{
  PROMOTE_QUEEN,
  PROMOTE_ROOK,
  PROMOTE_BISHOP,
  PROMOTE_KNIGHT
};

struct LastMove
{
  Piece *piece;
  int fromRow;
  char fromCol;
  int toRow;
  char toCol;
};

struct MoveHistory
{
  int fromRow;
  char fromCol;
  int toRow;
  char toCol;
  PieceType capturedType;
  PieceColor capturedColor;
  bool kingMoved;
  bool rookMoved;
  char rookCol;
};

class ChessBoard
{

  LastMove lastMove;

public:
  ChessBoard();

  // Game initialization
  void initializeStandardGame(); // Sets up standard chess starting position
  void clearBoard();             // Clears the board and resets game state

  void placePiece(Piece *piece, int row, char col);
  void removePiece(int row, char col);
  bool movePiece(int fromRow, char fromCol, int toRow, char toCol);
  bool movePiece(int fromRow, char fromCol, int toRow, char toCol, PromotionType promotionChoice); // With promotion choice
  void captureAndPlace(Piece *piece, int row, char col);
  bool isSquareAttacked(int row, char col, PieceColor attackerColor);

  Piece *getPiece(int row, char col);
  void printBoard();
  void promotePawn(int row, char col, PromotionType promoteChoice, PieceColor color);

  // Game state methods
  bool isInCheck(PieceColor color);
  bool isCheckmate(PieceColor color);
  bool isStalemate(PieceColor color);
  bool isDraw();
  GameState getGameState();
  PieceColor getCurrentTurn();
  void setCurrentTurn(PieceColor color);

  // Helper methods
  bool findKing(PieceColor color, int &row, char &col);
  bool isPathClear(int fromRow, char fromCol, int toRow, char toCol);
  bool isMoveLegal(int fromRow, char fromCol, int toRow, char toCol);
  bool hasAnyValidMove(PieceColor color);
  int countMoveRepetitions();

private:
  Piece *board[8][8];
  PieceColor currentTurn;
  GameState gameState;
  MoveHistory moveHistory[6]; // Store last 3 moves from each side (6 total)
  int moveCount;              // Current number of moves stored (max 6)
  int halfMoveClock;          // For 50-move rule

  // Board state storage for repetition detection
  // Store compact representation of board state (piece positions)
  struct BoardState
  {
    char state[64];  // 8x8 board, each char represents a piece: ' '=empty, 'P'=pawn, etc.
    PieceColor turn; // Whose turn it was
  };
  BoardState positionHistory[6]; // Store last 6 positions (for threefold repetition check)
  int positionCount;

  int rowToIndex(int row);
  int colToIndex(char col);
  char indexToCol(int index);
  int indexToRow(int index);
  void addToHistory(int fromRow, char fromCol, int toRow, char toCol);
  void storeBoardState(); // Store current position for repetition detection
  bool compareBoardStates(const BoardState &state1, const BoardState &state2);
  void updateGameState();
  bool wouldMoveLeaveKingInCheck(int fromRow, char fromCol, int toRow, char toCol, PieceColor color);
};

#endif
