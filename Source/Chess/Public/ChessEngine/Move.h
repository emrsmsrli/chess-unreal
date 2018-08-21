// Copyright 2018 Emre Simsirli

#pragma once

#include "UnrealString.h"

class TMove {
public:
    static const uint32 flag_en_passant = 0x40000;
    static const uint32 flag_pawn_start = 0x80000;
    static const uint32 flag_castling = 0x1000000;

    static const TMove no_move;

private:
    uint32 move_;
    uint32 score_ = 0;
    explicit TMove(uint32 m);

public:
	TMove() : TMove(0) {}

    uint32 from() const;
    uint32 to() const;

    uint32 captured_piece() const;
    uint32 promoted_piece() const;

    bool is_captured() const;
    bool is_promoted() const;
    bool is_enpassant() const;
    bool is_pawnstart() const;
    bool is_castling() const;

    void set_score(uint32 s);
    uint32 score() const;

    static TMove create(uint32 from, uint32 to, uint32 captured, uint32 promoted, uint32 flags);

    FString ToString() const;
    bool operator==(const TMove& o) const;
    bool operator!=(const TMove& o) const;

private:
    uint32 get(uint32 shift, uint32 and) const;
};
