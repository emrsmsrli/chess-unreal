// Copyright 2018 Emre Simsirli

#include "ChessEngine.h"
#include "Bitboard.h"
#include "Board.h"
#include "PosKey.h"
#include "Square.h"

TChessEngine::TChessEngine() {}

void TChessEngine::Initialize()
{
    ESquare::Initialize();
    TBitboard::Initialize();
    PosKey::Initialize();
    TMoveGenerator::Initialize();
}
