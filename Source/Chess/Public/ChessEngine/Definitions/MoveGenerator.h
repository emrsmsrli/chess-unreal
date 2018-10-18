// Copyright 2018 Emre Simsirli

#pragma once

#include "Containers/Array.h"
#include "Move.h"

class TMoveGenerator
{
    class TBoard* ref_;

public:
    explicit TMoveGenerator(class TBoard* ref);

    TArray<TMove> generate_moves();
    TArray<TMove> generate_moves(uint32 sq);

    static void Initialize();

private:
    void generate_pawn_moves(uint32 sq, TArray<TMove>& moves);
    void generate_sliding_moves(uint32 sq, TArray<TMove>& moves);
    void generate_non_sliding_moves(uint32 sq, TArray<TMove>& moves);
    void generate_castling_moves(TArray<TMove>& moves);

    void add_quiet_move(TMove move, TArray<TMove>& moves);
    void add_capture_move(TMove move, TArray<TMove>& moves);

    void add_pawn_regular_move(uint32 from, uint32 to, TArray<TMove>& moves);
    void add_pawn_capture_move(uint32 from, uint32 to,
                               uint32 captured, TArray<TMove>& moves);
    void add_en_passant_move(TMove move, TArray<TMove>& moves);
};
