// Copyright 2018 Emre Simsirli

#include "ChessEngine/ChessEngine.h"
#include "ChessEngine/Board/Bitboard.h"
#include "ChessEngine/Definitions/PosKey.h"
#include "ChessEngine/Definitions/Transition.h"
#include "ChessEngine/Definitions/Square.h"

TChessEngine::TChessEngine() {

}

void TChessEngine::Initialize() {
    TBitboard::Initialize();
    PosKey::Initialize();
    Transition::Initialize();
    Square::Initialize();
}
