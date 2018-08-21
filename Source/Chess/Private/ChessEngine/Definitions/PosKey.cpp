// Copyright 2018 Emre Simsirli

#include "ChessEngine/Definitions/PosKey.h"
#include "ChessEngine/Definitions/Consts.h"
#include "UnrealMathUtility.h"

#define RAND_64() 	((uint64)FMath::Rand() | \
					(uint64)FMath::Rand() << 15 | \
					(uint64)FMath::Rand() << 30 | \
					(uint64)FMath::Rand() << 45 | \
					((uint64)FMath::Rand() & 0xf) << 60 )

namespace {
    uint64 piece_keys[n_pieces][n_board_squares_x];
    uint64 side_key;
    uint64 castle_keys[16];
}

void PosKey::Initialize() {
    for(auto& piece_key : piece_keys)
        for(auto& j : piece_key)
            j = RAND_64();

    side_key = RAND_64();

    for(auto& castle_key : castle_keys) {
        castle_key = RAND_64();
	}
}

uint64 PosKey::GetPieceKey(const uint32 piece_number, const uint32 square) {
    return piece_keys[piece_number][square];
}

uint64 PosKey::GetSideKey() {
    return side_key;
}

uint64 PosKey::GetCastleKey(const uint32 permission) {
    return castle_keys[permission];
}
