#include "ChessBoard.h"
#include "King.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"
#include "Pawn.h"
#include <Arduino.h>

ChessBoard::ChessBoard()
{
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            board[r][c] = nullptr;
        }
    }
    currentTurn = WHITE;
    gameState = GAME_ACTIVE;
    moveCount = 0;
    halfMoveClock = 0;
    positionCount = 0;
    lastMove.piece = nullptr;
}

int ChessBoard::rowToIndex(int row) { return row - 1; }
int ChessBoard::colToIndex(char col) { return col - 'A'; }
char ChessBoard::indexToCol(int index) { return 'A' + index; }
int ChessBoard::indexToRow(int index) { return index + 1; }

Piece *ChessBoard::getPiece(int row, char col)
{
    return board[rowToIndex(row)][colToIndex(col)];
}

void ChessBoard::placePiece(Piece *piece, int row, char col)
{
    board[rowToIndex(row)][colToIndex(col)] = piece;
}

void ChessBoard::removePiece(int row, char col)
{
    Piece *p = getPiece(row, col);
    if (p != nullptr)
    {
        Serial.print("Removing piece: ");
        p->printInfo();
        delete p;
        board[rowToIndex(row)][colToIndex(col)] = nullptr;
    }
}
void ChessBoard::captureAndPlace(Piece *piece, int row, char col)
{
    if (getPiece(row, col) != nullptr)
    {
        removePiece(row, col);
    }
    placePiece(piece, row, col);
}

// Move piece with promotion choice (for pawns)
bool ChessBoard::movePiece(int fromRow, char fromCol, int toRow, char toCol, PromotionType promotionChoice)
{
    // Check if game is over
    if (gameState != GAME_ACTIVE)
    {
        Serial.println("Game is over!");
        return false;
    }

    Piece *piece = getPiece(fromRow, fromCol);
    if (!piece)
    {
        Serial.println("No piece to move!");
        return false;
    }

    // Check if it's the correct player's turn
    if (piece->getColor() != currentTurn)
    {
        Serial.println("Not your turn!");
        return false;
    }

    // Check if the piece can move (pattern-wise)
    if (!piece->canMove(fromRow, fromCol, toRow, toCol))
    {
        Serial.println("Illegal move for this piece!");
        return false;
    }

    // Check if trying to capture own piece (not allowed)
    Piece *targetPiece = getPiece(toRow, toCol);
    if (targetPiece != nullptr && targetPiece->getColor() == piece->getColor())
    {
        Serial.println("Cannot capture your own piece!");
        return false;
    }

    // Check if path is clear (for pieces that need it)
    if (!isPathClear(fromRow, fromCol, toRow, toCol))
    {
        Serial.println("Path is blocked!");
        return false;
    }

    // Check if move is legal (doesn't leave own king in check)
    if (!isMoveLegal(fromRow, fromCol, toRow, toCol))
    {
        Serial.println("Move would leave king in check!");
        return false;
    }

    // If in check, verify this move gets out of check
    if (isInCheck(currentTurn))
    {
        if (wouldMoveLeaveKingInCheck(fromRow, fromCol, toRow, toCol, currentTurn))
        {
            // This means the move would still leave us in check, so it's illegal
            Serial.println("Must get out of check!");
            return false;
        }
    }

    // Store move info for history (before the move)
    Piece *capturedPiece = getPiece(toRow, toCol);

    // --- Special moves ---

    // King
    bool kingMoved = false;
    bool rookMoved = false;
    char rookCol = 'A';
    if (piece->getTypeName()[0] == 'K')
    {
        King *k = (King *)piece;
        kingMoved = k->hasMoved;

        // Castling - additional validation
        if (abs(toCol - fromCol) == 2)
        {
            // Cannot castle if in check
            if (isInCheck(currentTurn))
            {
                Serial.println("Cannot castle while in check!");
                return false;
            }

            rookCol = (toCol > fromCol) ? 'H' : 'A';
            Piece *rookPiece = getPiece(fromRow, rookCol);
            if (!rookPiece || rookPiece->getTypeName()[0] != 'R')
            {
                Serial.println("No rook to castle with!");
                return false;
            }

            // Check if rook is same color as king
            if (rookPiece->getColor() != k->getColor())
            {
                Serial.println("Cannot castle with opponent's rook!");
                return false;
            }

            Rook *r = (Rook *)rookPiece;

            // Check if king and rook haven't moved
            if (k->hasMoved || r->hasMoved)
            {
                Serial.println("Cannot castle - king or rook has moved!");
                return false;
            }

            rookMoved = r->hasMoved;

            // Check if squares between king and rook are clear
            char step = (toCol > fromCol) ? 1 : -1;
            for (char c = fromCol + step; c != rookCol; c += step)
            {
                if (getPiece(fromRow, c) != nullptr)
                {
                    Serial.println("Cannot castle - path is blocked!");
                    return false;
                }
            }

            // Check if squares the king moves through are attacked
            for (char c = fromCol; c != toCol + step; c += step)
            {
                if (isSquareAttacked(fromRow, c, (currentTurn == WHITE) ? BLACK : WHITE))
                {
                    Serial.println("Cannot castle through check!");
                    return false;
                }
            }

            char newRookCol = (toCol > fromCol) ? 'F' : 'D';
            placePiece(r, fromRow, newRookCol);
            removePiece(fromRow, rookCol);

            r->hasMoved = true;
        }

        k->hasMoved = true;
    }

    // Rook
    if (piece->getTypeName()[0] == 'R')
    {
        Rook *r = (Rook *)piece;
        r->hasMoved = true;
    }

    // Pawn
    bool pawnMoved = false;
    if (piece->getTypeName()[0] == 'P')
    {
        Pawn *pawn = (Pawn *)piece;
        int dir = (pawn->getColor() == WHITE) ? 1 : -1;
        pawnMoved = true;

        // En passant
        if (abs(toCol - fromCol) == 1 && toRow - fromRow == dir && getPiece(toRow, toCol) == nullptr)
        {
            if (lastMove.piece && lastMove.piece->getTypeName()[0] == 'P')
            {
                // Check if last move was by opponent's pawn (different color)
                if (lastMove.piece->getColor() != pawn->getColor() &&
                    abs(lastMove.toRow - lastMove.fromRow) == 2 &&
                    lastMove.toRow == fromRow &&
                    lastMove.toCol == toCol)
                {
                    removePiece(lastMove.toRow, lastMove.toCol);
                }
            }
        }

        // Update last move for en passant (only if pawn advanced two squares)
        if (abs(toRow - fromRow) == 2)
        {
            lastMove.piece = pawn;
            lastMove.fromRow = fromRow;
            lastMove.fromCol = fromCol;
            lastMove.toRow = toRow;
            lastMove.toCol = toCol;
        }
        else
        {
            // Clear en passant opportunity if pawn didn't advance two squares
            lastMove.piece = nullptr;
        }

        // Reset half-move clock on pawn move
        halfMoveClock = 0;
    }
    else
    {
        // Non-pawn move: clear en passant opportunity
        lastMove.piece = nullptr;

        if (capturedPiece)
        {
            // Reset half-move clock on capture
            halfMoveClock = 0;
        }
        else
        {
            // Increment half-move clock
            halfMoveClock++;
        }
    }

    // --- Move the piece ---
    captureAndPlace(piece, toRow, toCol);
    board[rowToIndex(fromRow)][colToIndex(fromCol)] = nullptr;

    // --- Pawn promotion with choice ---
    if (piece->getTypeName()[0] == 'P')
    {
        int finalRow = (piece->getColor() == WHITE) ? 8 : 1;
        if (toRow == finalRow)
        {
            promotePawn(toRow, toCol, promotionChoice, piece->getColor());
        }
    }

    // Add to move history
    addToHistory(fromRow, fromCol, toRow, toCol);

    // Switch turn
    currentTurn = (currentTurn == WHITE) ? BLACK : WHITE;

    // Store board state for repetition detection (after turn switch)
    storeBoardState();

    // Update game state
    updateGameState();

    return true;
}

// Move piece (default version, pawns promote to queen)
bool ChessBoard::movePiece(int fromRow, char fromCol, int toRow, char toCol)
{
    return movePiece(fromRow, fromCol, toRow, toCol, PROMOTE_QUEEN);
}

void ChessBoard::printBoard()
{
    for (int r = 8; r >= 1; r--)
    {
        Serial.print(r);
        Serial.print(" ");
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p == nullptr)
                Serial.print(". ");
            else
                Serial.print(p->getTypeName()[0]), Serial.print(" ");
        }
        Serial.println();
    }
    Serial.println("  A B C D E F G H");
}

bool ChessBoard::isSquareAttacked(int row, char col, PieceColor attackerColor)
{
    for (int r = 1; r <= 8; r++)
    {
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p != nullptr && p->getColor() == attackerColor)
            {
                if (p->canMove(r, c, row, col))
                {
                    // Check if path is clear (for pieces that need it)
                    if (isPathClear(r, c, row, col))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Helper method to check if path between two squares is clear
bool ChessBoard::isPathClear(int fromRow, char fromCol, int toRow, char toCol)
{
    // Knights don't need path checking
    Piece *piece = getPiece(fromRow, fromCol);
    if (piece && piece->getTypeName()[0] == 'N')
    {
        return true;
    }

    // Kings moving one square don't need path checking
    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);
    if (rowDiff <= 1 && colDiff <= 1)
    {
        return true;
    }

    // Pawns need special handling
    if (piece && piece->getTypeName()[0] == 'P')
    {
        // For diagonal captures, check if target square has enemy piece
        if (colDiff != 0)
        {
            Piece *targetPiece = getPiece(toRow, toCol);
            // If target square is empty, check for en passant
            if (targetPiece == nullptr)
            {
                // Check en passant condition
                int dir = (piece->getColor() == WHITE) ? 1 : -1;
                // En passant: diagonal move one square forward into empty square
                if (abs(toCol - fromCol) == 1 && toRow - fromRow == dir)
                {
                    // Check if last move was by opponent's pawn that moved two squares
                    if (lastMove.piece && lastMove.piece->getTypeName()[0] == 'P')
                    {
                        if (lastMove.piece->getColor() != piece->getColor() &&
                            abs(lastMove.toRow - lastMove.fromRow) == 2 &&
                            lastMove.toRow == fromRow &&
                            lastMove.toCol == toCol)
                        {
                            return true; // En passant is valid
                        }
                    }
                }
                return false; // Empty diagonal target without en passant is invalid
            }
            return targetPiece != nullptr && targetPiece->getColor() != piece->getColor();
        }
        // For forward moves, check if path is clear and destination is empty
        if (getPiece(toRow, toCol) != nullptr)
        {
            return false; // Destination must be empty for forward moves
        }
        int dir = (toRow > fromRow) ? 1 : -1;
        for (int r = fromRow + dir; r != toRow; r += dir)
        {
            if (getPiece(r, fromCol) != nullptr)
            {
                return false;
            }
        }
        return true;
    }

    // For other pieces, check all squares in between
    int rowStep = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1
                                                            : 0;
    int colStep = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1
                                                            : 0;

    int r = fromRow + rowStep;
    char c = fromCol + colStep;

    while (r != toRow || c != toCol)
    {
        if (getPiece(r, c) != nullptr)
        {
            return false;
        }
        r += rowStep;
        c += colStep;
    }

    return true;
}

// Find the king of the specified color
bool ChessBoard::findKing(PieceColor color, int &row, char &col)
{
    for (int r = 1; r <= 8; r++)
    {
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p != nullptr && p->getTypeName()[0] == 'K' && p->getColor() == color)
            {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

// Check if a color's king is in check
bool ChessBoard::isInCheck(PieceColor color)
{
    int kingRow;
    char kingCol;
    if (!findKing(color, kingRow, kingCol))
    {
        return false; // No king found (shouldn't happen in normal game)
    }

    PieceColor attackerColor = (color == WHITE) ? BLACK : WHITE;
    return isSquareAttacked(kingRow, kingCol, attackerColor);
}

// Check if a move would leave the king in check
bool ChessBoard::wouldMoveLeaveKingInCheck(int fromRow, char fromCol, int toRow, char toCol, PieceColor color)
{
    // Make a temporary move
    Piece *piece = getPiece(fromRow, fromCol);
    Piece *capturedPiece = getPiece(toRow, toCol);

    // Check if this is an en passant capture
    bool isEnPassant = false;
    Piece *enPassantCapturedPawn = nullptr;

    if (piece && piece->getTypeName()[0] == 'P')
    {
        Pawn *pawn = (Pawn *)piece;
        int dir = (pawn->getColor() == WHITE) ? 1 : -1;

        // En passant: pawn moves diagonally one square forward into empty square
        if (abs(toCol - fromCol) == 1 &&
            toRow - fromRow == dir &&
            getPiece(toRow, toCol) == nullptr)
        {
            // Check if there's an en passant-eligible pawn at (fromRow, toCol)
            if (lastMove.piece && lastMove.piece->getTypeName()[0] == 'P')
            {
                if (lastMove.piece->getColor() != pawn->getColor() &&
                    abs(lastMove.toRow - lastMove.fromRow) == 2 &&
                    lastMove.toRow == fromRow &&
                    lastMove.toCol == toCol)
                {
                    isEnPassant = true;
                    enPassantCapturedPawn = getPiece(fromRow, toCol);
                }
            }
        }
    }

    // Execute the move temporarily
    board[rowToIndex(toRow)][colToIndex(toCol)] = piece;
    board[rowToIndex(fromRow)][colToIndex(fromCol)] = nullptr;

    // If en passant, also clear the captured pawn's square
    if (isEnPassant && enPassantCapturedPawn)
    {
        board[rowToIndex(fromRow)][colToIndex(toCol)] = nullptr;
    }

    // Check if king is in check
    bool inCheck = isInCheck(color);

    // Undo the move
    board[rowToIndex(fromRow)][colToIndex(fromCol)] = piece;
    board[rowToIndex(toRow)][colToIndex(toCol)] = capturedPiece;

    // If en passant, restore the captured pawn
    if (isEnPassant && enPassantCapturedPawn)
    {
        board[rowToIndex(fromRow)][colToIndex(toCol)] = enPassantCapturedPawn;
    }

    return inCheck;
}

// Check if a move is legal (doesn't leave own king in check)
bool ChessBoard::isMoveLegal(int fromRow, char fromCol, int toRow, char toCol)
{
    Piece *piece = getPiece(fromRow, fromCol);
    if (!piece)
        return false;

    // Check if the move would leave our own king in check
    return !wouldMoveLeaveKingInCheck(fromRow, fromCol, toRow, toCol, piece->getColor());
}

// Check if a color has any valid moves
bool ChessBoard::hasAnyValidMove(PieceColor color)
{
    for (int r = 1; r <= 8; r++)
    {
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p != nullptr && p->getColor() == color)
            {
                // Try all possible destination squares
                for (int toRow = 1; toRow <= 8; toRow++)
                {
                    for (char toCol = 'A'; toCol <= 'H'; toCol++)
                    {
                        if (p->canMove(r, c, toRow, toCol) &&
                            isPathClear(r, c, toRow, toCol) &&
                            isMoveLegal(r, c, toRow, toCol))
                        {
                            // Check if in check - if so, verify this move gets out
                            if (isInCheck(color))
                            {
                                if (!wouldMoveLeaveKingInCheck(r, c, toRow, toCol, color))
                                {
                                    return true;
                                }
                            }
                            else
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

// Check if it's checkmate
bool ChessBoard::isCheckmate(PieceColor color)
{
    return isInCheck(color) && !hasAnyValidMove(color);
}

// Check if it's stalemate
bool ChessBoard::isStalemate(PieceColor color)
{
    return !isInCheck(color) && !hasAnyValidMove(color);
}

// Store current board state in history
void ChessBoard::storeBoardState()
{
    // Shift history if full (remove oldest)
    if (positionCount >= 50)
    {
        for (int i = 0; i < 49; i++)
        {
            positionHistory[i] = positionHistory[i + 1];
        }
        positionCount = 49;
    }

    // Add the new state
    BoardState &state = positionHistory[positionCount];
    state.turn = currentTurn;

    // Encode board state: ' ' = empty, piece type char = piece
    // For pieces, use lowercase for black, uppercase for white
    int idx = 0;
    for (int r = 1; r <= 8; r++)
    {
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p == nullptr)
            {
                state.state[idx] = ' ';
            }
            else
            {
                char pieceChar = p->getTypeName()[0]; // P, R, N, B, Q, K
                if (p->getColor() == BLACK)
                {
                    state.state[idx] = pieceChar + 32; // lowercase for black
                }
                else
                {
                    state.state[idx] = pieceChar; // uppercase for white
                }
            }
            idx++;
        }
    }
    positionCount++;
}

// Compare two board states
bool ChessBoard::compareBoardStates(const BoardState &state1, const BoardState &state2)
{
    // Must be same turn
    if (state1.turn != state2.turn)
        return false;

    // Compare all squares
    for (int i = 0; i < 64; i++)
    {
        if (state1.state[i] != state2.state[i])
            return false;
    }

    return true;
}

// Count how many times the current position has occurred
int ChessBoard::countMoveRepetitions()
{
    if (positionCount < 2)
        return 1; // Current position is first occurrence

    // Create current board state
    BoardState currentState;
    currentState.turn = currentTurn;

    int idx = 0;
    for (int r = 1; r <= 8; r++)
    {
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p == nullptr)
            {
                currentState.state[idx] = ' ';
            }
            else
            {
                char pieceChar = p->getTypeName()[0];
                if (p->getColor() == BLACK)
                {
                    currentState.state[idx] = pieceChar + 32;
                }
                else
                {
                    currentState.state[idx] = pieceChar;
                }
            }
            idx++;
        }
    }

    // Count how many times this position occurred
    // Note: storeBoardState already appended the current state to positionHistory,
    // so we start count at 0 and compare against all stored entries
    int count = 0;
    for (int i = 0; i < positionCount; i++)
    {
        if (compareBoardStates(currentState, positionHistory[i]))
        {
            count++;
        }
    }

    return count;
}

// Check if game is a draw
bool ChessBoard::isDraw()
{
    // 50-move rule
    if (halfMoveClock >= 100)
    {
        return true;
    }

    if (countMoveRepetitions() >= 3)
    {
        return true;
    }

    // Stalemate
    if (isStalemate(currentTurn))
    {
        return true;
    }

    // Insufficient material check
    // A game is drawn if neither side has sufficient material to force checkmate
    int whitePawns = 0, blackPawns = 0;
    int whiteRooks = 0, blackRooks = 0;
    int whiteKnights = 0, blackKnights = 0;
    int whiteBishops = 0, blackBishops = 0;
    int whiteQueens = 0, blackQueens = 0;
    bool whiteHasLightBishop = false, whiteHasDarkBishop = false;
    bool blackHasLightBishop = false, blackHasDarkBishop = false;

    for (int r = 1; r <= 8; r++)
    {
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p != nullptr)
            {
                char type = p->getTypeName()[0];
                // Determine if square is light or dark
                // A1 is dark, so even sum = dark, odd sum = light
                // A=1, B=2, etc., rows 1-8
                int colVal = c - 'A' + 1;
                bool isLightSquare = ((r + colVal) % 2 == 1);

                if (p->getColor() == WHITE)
                {
                    if (type == 'P')
                        whitePawns++;
                    else if (type == 'R')
                        whiteRooks++;
                    else if (type == 'N')
                        whiteKnights++;
                    else if (type == 'B')
                    {
                        whiteBishops++;
                        if (isLightSquare)
                            whiteHasLightBishop = true;
                        else
                            whiteHasDarkBishop = true;
                    }
                    else if (type == 'Q')
                        whiteQueens++;
                }
                else
                {
                    if (type == 'P')
                        blackPawns++;
                    else if (type == 'R')
                        blackRooks++;
                    else if (type == 'N')
                        blackKnights++;
                    else if (type == 'B')
                    {
                        blackBishops++;
                        if (isLightSquare)
                            blackHasLightBishop = true;
                        else
                            blackHasDarkBishop = true;
                    }
                    else if (type == 'Q')
                        blackQueens++;
                }
            }
        }
    }

    // If either side has pawns, rooks, or queens, there is sufficient material
    if (whitePawns > 0 || blackPawns > 0 ||
        whiteRooks > 0 || blackRooks > 0 ||
        whiteQueens > 0 || blackQueens > 0)
    {
        return false;
    }

    // Count total minor pieces (knights + bishops) for each side
    int whiteMinorPieces = whiteKnights + whiteBishops;
    int blackMinorPieces = blackKnights + blackBishops;

    // King vs King - insufficient material
    if (whiteMinorPieces == 0 && blackMinorPieces == 0)
    {
        return true;
    }

    // King + one minor piece vs King - insufficient material
    if ((whiteMinorPieces == 0 && blackMinorPieces == 1) ||
        (whiteMinorPieces == 1 && blackMinorPieces == 0))
    {
        return true;
    }

    // King + two knights vs King - insufficient material (2 knights cannot force mate)
    if ((whiteMinorPieces == 0 && blackKnights == 2 && blackBishops == 0) ||
        (blackMinorPieces == 0 && whiteKnights == 2 && whiteBishops == 0))
    {
        return true;
    }

    // King + bishop vs King + bishop - insufficient material only if same-color bishops
    if (whiteMinorPieces == 1 && blackMinorPieces == 1 &&
        whiteBishops == 1 && blackBishops == 1 && whiteKnights == 0 && blackKnights == 0)
    {
        // Check if both bishops are on same color squares
        bool whiteOnLight = whiteHasLightBishop;
        bool blackOnLight = blackHasLightBishop;
        if (whiteOnLight == blackOnLight)
        {
            return true; // Same-color bishops cannot force mate
        }
        // Opposite-color bishops can force mate, so not insufficient material
    }

    // King + knight vs King + knight - insufficient material
    if (whiteMinorPieces == 1 && blackMinorPieces == 1 &&
        whiteKnights == 1 && blackKnights == 1 && whiteBishops == 0 && blackBishops == 0)
    {
        return true;
    }

    // King + bishop vs King + knight - insufficient material
    if ((whiteMinorPieces == 1 && blackMinorPieces == 1) &&
        ((whiteBishops == 1 && blackKnights == 1) || (whiteKnights == 1 && blackBishops == 1)))
    {
        return true;
    }

    // All other combinations have sufficient material
    return false;
}

// Get current game state
GameState ChessBoard::getGameState()
{
    return gameState;
}

// Update game state based on current position
void ChessBoard::updateGameState()
{
    if (isCheckmate(WHITE))
    {
        gameState = GAME_CHECKMATE_WHITE;
    }
    else if (isCheckmate(BLACK))
    {
        gameState = GAME_CHECKMATE_BLACK;
    }
    else if (isStalemate(currentTurn))
    {
        gameState = GAME_STALEMATE;
    }
    else if (isDraw())
    {
        gameState = GAME_DRAW;
    }
    else
    {
        gameState = GAME_ACTIVE;
    }
}

// Get current turn
PieceColor ChessBoard::getCurrentTurn()
{
    return currentTurn;
}

// Set current turn
void ChessBoard::setCurrentTurn(PieceColor color)
{
    currentTurn = color;
}

// Add move to history
void ChessBoard::addToHistory(int fromRow, char fromCol, int toRow, char toCol)
{
    if (moveCount < 100)
    {
        moveHistory[moveCount].fromRow = fromRow;
        moveHistory[moveCount].fromCol = fromCol;
        moveHistory[moveCount].toRow = toRow;
        moveHistory[moveCount].toCol = toCol;
        moveCount++;
    }
}

// Clear the board and reset game state
void ChessBoard::clearBoard()
{
    // Delete all pieces on the board
    for (int r = 1; r <= 8; r++)
    {
        for (char c = 'A'; c <= 'H'; c++)
        {
            Piece *p = getPiece(r, c);
            if (p != nullptr)
            {
                delete p;
                board[rowToIndex(r)][colToIndex(c)] = nullptr;
            }
        }
    }

    // Reset game state
    currentTurn = WHITE;
    gameState = GAME_ACTIVE;
    moveCount = 0;
    halfMoveClock = 0;
    positionCount = 0;
    lastMove.piece = nullptr;
}

// Initialize standard chess starting position
void ChessBoard::initializeStandardGame()
{
    // Clear any existing pieces
    clearBoard();

    // Place white pieces
    placePiece(new Rook(WHITE), 1, 'A');
    placePiece(new Knight(WHITE), 1, 'B');
    placePiece(new Bishop(WHITE), 1, 'C');
    placePiece(new Queen(WHITE), 1, 'D');
    placePiece(new King(WHITE), 1, 'E');
    Piece *newPiece = nullptr;
    switch (promoteChoice)
    {
    case PROMOTE_QUEEN:
        newPiece = new Queen(color);
        break;
    case PROMOTE_ROOK:
        newPiece = new Rook(color);
        break;
    case PROMOTE_BISHOP:
        newPiece = new Bishop(color);
        break;
    case PROMOTE_KNIGHT:
        newPiece = new Knight(color);
        break;
    default:
        Serial.println("Invalid promotion choice, defaulting to Queen");
        newPiece = new Queen(color);
        break;
    }
    placePiece(new Bishop(BLACK), 8, 'F');
    placePiece(new Knight(BLACK), 8, 'G');
    placePiece(new Rook(BLACK), 8, 'H');

    // Place black pawns
    for (char c = 'A'; c <= 'H'; c++)
    {
        placePiece(new Pawn(BLACK), 7, c);
    }

    // Store initial board state for repetition detection
    storeBoardState();

    Serial.println("Standard chess game initialized!");
}

void ChessBoard::promotePawn(int row, char col, PromotionType promoteChoice, PieceColor color)
{
    Piece *pawn = getPiece(row, col);
    if (!pawn || pawn->getTypeName()[0] != 'P')
    {
        Serial.println("No pawn to promote on this square!");
        return;
    }

    removePiece(row, col);

    Piece *newPiece = nullptr;
    switch (promoteChoice)
    {
    case PROMOTE_QUEEN:
        newPiece = new Queen(color);
        break;
    case PROMOTE_ROOK:
        newPiece = new Rook(color);
        break;
    case PROMOTE_BISHOP:
        newPiece = new Bishop(color);
        break;
    case PROMOTE_KNIGHT:
        newPiece = new Knight(color);
        break;
    }

    if (newPiece)
    {
        placePiece(newPiece, row, col);
        Serial.print("Pawn promoted to ");
        Serial.print(newPiece->getTypeName());
        Serial.println();
    }
}