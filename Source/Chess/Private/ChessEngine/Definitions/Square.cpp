// Copyright 2018 Emre Simsirli

#include "Square.h"
#include "Consts.h"

namespace
{
    uint32 files[n_board_squares_x];
    uint32 ranks[n_board_squares_x];

    uint32 sq120_sq64[n_board_squares_x];
    uint32 sq64_sq120[n_board_squares];

    char files_c[] = "abcdefgh";
    char ranks_c[] = "12345678";
}

FString EFile::AsString(const uint32 file)
{
    return FString::Printf(TEXT("%c"), files_c[file]);
}

FString ERank::AsString(const uint32 rank)
{
    return FString::Printf(TEXT("%c"), ranks_c[rank]);
}

uint32 ESquare::Rank(const uint32 sq)
{
    return ranks[sq];
}

uint32 ESquare::File(const uint32 sq)
{
    return files[sq];
}

uint32 ESquare::Sq120(const uint32 file, const uint32 rank)
{
    return 21 + file + rank * 10;
}

uint32 ESquare::Sq64(const uint32 file, const uint32 rank)
{
    return Sq64(Sq120(file, rank));
}

uint32 ESquare::Sq120(const uint32 sq64)
{
    return sq64_sq120[sq64];
}

uint32 ESquare::Sq64(const uint32 sq120)
{
    return sq120_sq64[sq120];
}

FString ESquare::AsString(const uint32 sq)
{
    return EFile::AsString(File(sq)) + ERank::AsString(Rank(sq));
}

void ESquare::Initialize()
{
    for(uint32 i = 0; i < n_board_squares_x; ++i) {
        files[i] = ESquare::offboard;
        ranks[i] = ESquare::offboard;

        sq120_sq64[i] = ESquare::offboard;
    }

    uint32 sq64 = 0;
    for(uint32 r = ERank::rank_1; r < ERank::rank_none; ++r) {
        for(uint32 f = EFile::file_a; f < EFile::file_none; ++f) {
            const auto sq = Sq120(f, r);

            files[sq] = f;
            ranks[sq] = r;

            sq64_sq120[sq64] = sq;
            sq120_sq64[sq] = sq64;
            sq64++;
        }
    }
}
