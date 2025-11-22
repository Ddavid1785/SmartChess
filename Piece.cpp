#include "Piece.h"
#include <Arduino.h>

Piece::Piece(PieceType type, PieceColor color)
    : _type(type), _color(color) {}

PieceColor Piece::getColor() {
    return _color;
}

void Piece::printInfo() {
    Serial.print(getTypeName());
    Serial.print(" (");
    Serial.print(_color == WHITE ? "White" : "Black");
    Serial.println(")");
}
