// Copyright 2018 Emre Simsirli

#include "ChessEngine/Definitions/Square.h"
#include "ChessEngine/Definitions/Consts.h"
#include "ChessEngine/Definitions/BoardDefs.h"
#include "ChessEngine/Definitions/Transition.h"

namespace {
    uint32 files[n_board_squares_x];
    uint32 ranks[n_board_squares_x];
}

uint32 Square::Rank(const uint32 sq) {
    return ranks[sq];
}

uint32 Square::File(const uint32 sq) {
    return files[sq];
}

void Square::Initialize() {
    for(uint32 i = 0; i < n_board_squares_x; ++i) {
        files[i] = ESquare::offboard;
        ranks[i] = ESquare::offboard;
    }

    for(uint32 r = ERank::rank_1; r < ERank::rank_none; ++r) {
        for(uint32 f = EFile::file_a; f < EFile::file_none; ++f) {
            const auto sq = Transition::fr_sq120(f, r);
            files[sq] = f;
            ranks[sq] = r;
        }
    }
}
