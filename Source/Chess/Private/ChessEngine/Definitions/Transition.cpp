// Copyright 2018 Emre Simsirli

#include "ChessEngine/Definitions/Transition.h"
#include "ChessEngine/Definitions/Consts.h"
#include "ChessEngine/Definitions/BoardDefs.h"
#include "ChessEngine/Definitions/Square.h"

namespace {
    uint32 sq120_sq64[n_board_squares_x];
    uint32 sq64_sq120[n_board_squares];
}

uint32 Transition::fr_sq120(const uint32 file, const uint32 rank) {
    return 21 + file + rank * 10;
}

uint32 Transition::fr_sq64(const uint32 file, const uint32 rank) {
    return sq64(fr_sq120(file, rank));
}

uint32 Transition::sq120(const uint32 sq64) {
    return sq64_sq120[sq64];
}

uint32 Transition::sq64(const uint32 sq120) {
    return sq120_sq64[sq120];
}

void Transition::Initialize() {
    for(uint32 i = 0; i < n_board_squares_x; ++i) {
        sq120_sq64[i] = ESquare::offboard;
    }

    uint32 sq64 = 0;
    for(uint32 r = ERank::rank_1; r < ERank::rank_none; ++r) {
        for(uint32 f = EFile::file_a; f < EFile::file_none; ++f) {
            const auto sq = fr_sq120(f, r);
            sq64_sq120[sq64] = sq;
            sq120_sq64[sq] = sq64;
            sq64++;
        }
    }
}
