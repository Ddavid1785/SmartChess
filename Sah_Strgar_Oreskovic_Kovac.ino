#include "ChessBoard.h"
#include "Pawn.h"
#include "Knight.h"

ChessBoard board;

void setup() {
    Serial.begin(9600);

    Pawn* p = new Pawn(WHITE);
    Knight* k = new Knight(BLACK);

    board.placePiece(p, 2, 'B');
    board.placePiece(k, 8, 'G'); 

    board.printBoard();

    if(p->canMove(2,'B',4,'B')) Serial.println("Pawn can move 2 spaces");
    if(k->canMove(8,'G',6,'H')) Serial.println("Knight can move to H6");
    if(!p->canMove(2,'B',3,'C')) Serial.println("Pawn cannot move diagonally yet");
}

void loop() {}
