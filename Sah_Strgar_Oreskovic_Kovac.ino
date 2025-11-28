#include "ChessBoard.h"
#include "Pawn.h"
#include "Knight.h"
#include "Rook.h"
#include "Bishop.h"
#include "Queen.h"
#include "King.h"

ChessBoard board;

/*
 * ============================================================================
 * CHESS BOARD API - MAIN FUNCTIONS YOU'LL USE
 * ============================================================================
 *
 * These are the functions you'll actually call in your Arduino code.
 * Helper functions (like isCheckmate, isStalemate, etc.) are used internally
 * by the board and you don't need to call them directly.
 *
 * ============================================================================
 * 1. GAME INITIALIZATION
 * ============================================================================
 *
 * board.initializeStandardGame();
 *   - Sets up a standard chess starting position
 *   - Resets game state to GAME_ACTIVE
 *   - Sets current turn to WHITE
 *   - Call this once at the start of a new game
 *
 * ============================================================================
 * 2. MAKING MOVES
 * ============================================================================
 *
 * bool success = board.movePiece(fromRow, fromCol, toRow, toCol);
 *   - Makes a move from one square to another
 *   - Returns true if move was legal and executed, false otherwise
 *   - Automatically handles: turn validation, check, special moves, promotion
 *   - Example: board.movePiece(2, 'E', 4, 'E'); // White pawn e2 to e4
 *
 * bool success = board.movePiece(fromRow, fromCol, toRow, toCol, promotionChoice);
 *   - Same as above, but with pawn promotion choice
 *   - Use when pawn reaches 8th rank (white) or 1st rank (black)
 *   - promotionChoice: PROMOTE_QUEEN, PROMOTE_ROOK, PROMOTE_BISHOP, or PROMOTE_KNIGHT
 *   - Example: board.movePiece(7, 'A', 8, 'A', PROMOTE_QUEEN);
 *
 * ============================================================================
 * 3. CHECKING GAME STATE
 * ============================================================================
 *
 * GameState state = board.getGameState();
 *   - Returns current game state
 *   - Possible values:
 *     * GAME_ACTIVE - game is ongoing
 *     * GAME_CHECKMATE_WHITE - white is checkmated (black wins)
 *     * GAME_CHECKMATE_BLACK - black is checkmated (white wins)
 *     * GAME_STALEMATE - current player is stalemated (draw)
 *     * GAME_DRAW - game is a draw (50-move rule, threefold repetition, etc.)
 *   - Example: if (board.getGameState() == GAME_CHECKMATE_WHITE) { /* black wins */
**PieceColor turn = board.getCurrentTurn();
*-Returns whose turn it is : WHITE or BLACK * -Example : *if (board.getCurrentTurn() == WHITE){* // white's turn
                                                                                               *} *
                                          *bool inCheck = board.isInCheck(color);
*-Checks if a player's king is in check * -color : WHITE or BLACK * -Returns true if king is in check, false otherwise * -Example : *if (board.isInCheck(WHITE))
{
    *Serial.println("White is in check!");
    *
}
 * 
 * ============================================================================
 * 4. READING THE BOARD (for Arduino sensors)
 * ============================================================================
 * 
 * Piece* piece = board.getPiece(row, col);
 *   - Gets the piece at a specific square
 *   - Returns pointer to Piece if square has a piece, nullptr if empty
 *   - Use this to read what piece is on a square (for your sensors)
 *   - Example: 
 *     Piece* p = board.getPiece(1, 'E');
 *if (p != nullptr)
 {
     *if (p->getColor() == WHITE && p->getTypeName()[0] == 'K'){
         * // White king is on e1
         *}
         *
 }
 **= = == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
       *5. DISPLAY / DEBUGGING * ==
       == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
       **board.printBoard();
 *   - Prints the board to Serial (for debugging)
 *   - Shows pieces as single letters (K, Q, R, B, N, P)
 *   - Useful for testing and debugging
 * 
 * ============================================================================
 * EXAMPLE GAME FLOW
 * ============================================================================
 * 
 * void setup()
 {
     *Serial.begin(9600);
     ** // 1. Initialize a new game
      *board.initializeStandardGame();
     *board.printBoard(); // See starting position
     **                   // 2. Make moves
      *                   // White moves pawn e2 to e4
      *if (board.movePiece(2, 'E', 4, 'E'))
     {
         *Serial.println("White: e2-e4");
         *
     }
     else
     {
         *Serial.println("Illegal move!");
         *
     }
     ** // Black moves pawn e7 to e5
      *if (board.movePiece(7, 'E', 5, 'E'))
     {
         *Serial.println("Black: e7-e5");
         *
     }
     ** // White moves knight g1 to f3
      *board.movePiece(1, 'G', 3, 'F');
     ** // Check game state after each move
      *GameState state = board.getGameState();
     *if (state == GAME_ACTIVE)
     {
         *Serial.println("Game continues...");
         *
     }
     else if (state == GAME_CHECKMATE_WHITE)
     {
         *Serial.println("Black wins by checkmate!");
         *
     }
     else if (state == GAME_CHECKMATE_BLACK)
     {
         *Serial.println("White wins by checkmate!");
         *
     }
     else if (state == GAME_STALEMATE || state == GAME_DRAW)
     {
         *Serial.println("Game is a draw!");
         *
     }
     ** // 3. Check if a player is in check
      *if (board.isInCheck(WHITE))
     {
         *Serial.println("White is in check!");
         *
     }
     ** // 4. Check whose turn it is
      *PieceColor turn = board.getCurrentTurn();
     *if (turn == WHITE)
     {
         *Serial.println("White to move");
         *
     }
     else
     {
         *Serial.println("Black to move");
         *
     }
     ** // 5. Pawn promotion example (when pawn reaches 8th rank)
      * // board.movePiece(7, 'A', 8, 'A', PROMOTE_QUEEN); // Promote to queen
      * // board.movePiece(7, 'A', 8, 'A', PROMOTE_ROOK);  // Promote to rook
      * // board.movePiece(7, 'A', 8, 'A', PROMOTE_BISHOP); // Promote to bishop
      * // board.movePiece(7, 'A', 8, 'A', PROMOTE_KNIGHT); // Promote to knight
          *
 }
 * 
 * void loop() {
 *   // Your Arduino code here:
 *   // 1. Read sensors to detect piece movements
 *   // 2. Convert sensor data to fromRow/fromCol and toRow/toCol
 *   // 3. Call board.movePiece(fromRow, fromCol, toRow, toCol)
 *   // 4. Check board.getGameState() to see if game ended
 *   // 5. Update displays/LEDs based on game state
 * }
 * 
 * ============================================================================
 * NOTES FOR ARDUINO INTEGRATION
 * ============================================================================
 * 
 * 1. movePiece() does ALL validation automatically:
 *    - Checks if it's the correct player's turn
 *    - Validates piece movement patterns
 *    - Checks path blocking
 *    - Prevents leaving own king in check
 *    - Handles special moves (castling, en passant, promotion)
 *    - Updates game state after move
 * 
 * 2. You DON'T need to call these (they're used internally):
 *    - isCheckmate(), isStalemate(), isDraw() - use getGameState() instead
 *    - isMoveLegal(), isPathClear() - validation is automatic
 *    - clearBoard() - use initializeStandardGame() instead
 * 
 * 3. Coordinate system:
 *    - Rows: 1-8 (1 = bottom for white, 8 = top for white)
 *    - Columns: 'A'-'H' (A = left, H = right)
 *    - Example: e4 = row 4, column 'E'
 * 
 * 4. Error handling:
 *    - movePiece() returns false if move is illegal
 *    - Check return value and handle errors appropriately
 *    - Serial messages will print error reasons (for debugging)
 * 
 * ============================================================================
 */

void setup()
 {
     Serial.begin(9600);

     // Initialize a new chess game
     board.initializeStandardGame();

     // Print the starting board
     Serial.println("\n=== Starting Position ===");
     board.printBoard();

     // Example: Make a few moves
     Serial.println("\n=== Making Moves ===");

     // White: e2-e4
     if (board.movePiece(2, 'E', 4, 'E'))
     {
         Serial.println("White: e2-e4");
         board.printBoard();
     }

     // Black: e7-e5
     if (board.movePiece(7, 'E', 5, 'E'))
     {
         Serial.println("Black: e7-e5");
         board.printBoard();
     }

     // White: Ng1-f3
     if (board.movePiece(1, 'G', 3, 'F'))
     {
         Serial.println("White: Ng1-f3");
         board.printBoard();
     }

     // Check game state
     GameState state = board.getGameState();
     Serial.print("\nGame State: ");
     switch (state)
     {
     case GAME_ACTIVE:
         Serial.println("Active");
         break;
     case GAME_CHECKMATE_WHITE:
         Serial.println("Black wins!");
         break;
     case GAME_CHECKMATE_BLACK:
         Serial.println("White wins!");
         break;
     case GAME_STALEMATE:
         Serial.println("Stalemate (draw)");
         break;
     case GAME_DRAW:
         Serial.println("Draw");
         break;
     }

     // Check whose turn it is
     Serial.print("Current turn: ");
     Serial.println(board.getCurrentTurn() == WHITE ? "WHITE" : "BLACK");
 }

 void loop()
 {
     // Your Arduino integration code goes here:
     //
     // 1. Read sensors (magnetic, pressure, etc.) to detect piece positions
     // 2. Compare current positions with previous positions to detect moves
     // 3. Convert sensor coordinates to chess notation (row 1-8, col 'A'-'H')
     // 4. Call board.movePiece(fromRow, fromCol, toRow, toCol)
     // 5. If move returns false, handle error (illegal move, wrong turn, etc.)
     // 6. Check board.getGameState() after each move
     // 7. Update displays/LEDs/motors based on game state
     // 8. Handle pawn promotion (detect when pawn reaches 8th/1st rank)
     //
     // Example flow:
     // if (detectMove()) {
     //   int fromRow = getFromRow();
     //   char fromCol = getFromCol();
     //   int toRow = getToRow();
     //   char toCol = getToCol();
     //
     //   if (board.movePiece(fromRow, fromCol, toRow, toCol)) {
     //     // Move successful - update displays
     //     updateDisplay();
     //
     //     // Check if game ended
     //     GameState state = board.getGameState();
     //     if (state != GAME_ACTIVE) {
     //       handleGameEnd(state);
     //     }
     //   } else {
     //     // Illegal move - show error
     //     showError();
     //   }
     // }
 }
