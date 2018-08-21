// Copyright 2018 Emre Simsirli

#include "ChessEngine/Board/Bitboard.h"
#include "ChessEngine/Definitions/Consts.h"
#include "BoardDefs.h"
#include "Transition.h"

uint64 set_mask[n_board_squares];
uint64 clr_mask[n_board_squares];
uint32 btable[n_board_squares] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13,
    11, 53, 19, 34, 61, 29, 2, 51, 21, 43, 45, 10,
    18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49,
    5, 52, 26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39,
    48, 24, 59, 14, 12, 55, 38, 28, 58, 20, 37, 17, 36, 8
};

TBitboard::TBitboard() {
    board_ = 0;
}

void TBitboard::SetSquare(const uint32 sq) {
    board_ |= set_mask[sq];
}

void TBitboard::ClearSquare(const uint32 sq) {
    board_ &= clr_mask[sq];
}

uint32 TBitboard::Pop() {
    const auto b = board_ ^ (board_ - 1);
    const uint32 fold = (b & 0xFFFFFFFF) ^ (b >> 32);
    board_ &= board_ - 1;
    return btable[fold * 0x783a9b23 >> 26];
}

uint32 TBitboard::Count() const {
    auto board = board_;
    uint32 count = 0;
    while(board) {
        board &= board - 1;
        count++;
    }
    return count;
}

void TBitboard::Empty() {
    board_ = 0;
}

bool TBitboard::IsEmpty() const {
    return board_ == 0;
}

FString TBitboard::ToString() const {
    FString str;
    
    const uint64 s = 1L;
    for(int rank = ERank::rank_8; rank >= ERank::rank_1; --rank) {
        for(int file = EFile::file_a; file <= EFile::file_h; ++file) {
            const auto sq64 = Transition::fr_sq64(file, rank);

            if(s << sq64 & board_)
                str += "X";
            else
                str += "-";
        }
        str += "\n";
    }

    return str;
}

void TBitboard::Initialize() {
    for(uint32 i = 0; i < n_board_squares; ++i) {
        set_mask[i] = 1ULL << i;
        clr_mask[i] = ~set_mask[i];
    }
}
