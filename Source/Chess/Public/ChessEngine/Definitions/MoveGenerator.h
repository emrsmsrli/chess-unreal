// Copyright 2018 Emre Simsirli

#pragma once

#include "Containers/Array.h"
#include "Move.h"

class TMoveGenerator
{
    class TBoard* ref_;

public:
    explicit TMoveGenerator(class TBoard* ref);

    TArray<TMove> generate_moves() const;
    TArray<TMove> generate_moves(uint32 sq) const;
    bool does_move_exist(const TMove& m) const;

    static void Initialize();

private:
    void generate_pawn_moves(uint32 sq, TArray<TMove>& moves) const;
    void generate_sliding_moves(uint32 sq, TArray<TMove>& moves) const;
    void generate_non_sliding_moves(uint32 sq, TArray<TMove>& moves) const;
    void generate_castling_moves(TArray<TMove>& moves) const;

    void add_quiet_move(TMove move, TArray<TMove>& moves) const;
    void add_capture_move(TMove move, TArray<TMove>& moves) const;
    void add_en_passant_move(TMove move, TArray<TMove>& moves) const;

    void add_pawn_regular_move(uint32 from, uint32 to, TArray<TMove>& moves) const;
    void add_pawn_capture_move(uint32 from, uint32 to,
                               uint32 captured, TArray<TMove>& moves) const;
};
