// Copyright 2018 Emre Simsirli

#pragma once

#include "Bitboard.h"
#include "Move.h"
#include "Consts.h"
#include "Undo.h"
#include "PrincipleVariation.h"
#include "MoveGenerator.h"
#include "Evaluator.h"

class TBoard
{
    friend class TMoveGenerator;
    friend class TPrincipleVariationTable;
    friend class TEvaluator;

    uint32 b_[n_board_squares_x];
    TBitboard pawns_[3];
    uint32 king_sq_[2];
    uint32 en_passant_sq_;

    TArray<uint32> piece_locations_[n_pieces];

    uint32 n_big_pieces_[2]; // anything but pawn
    uint32 n_major_pieces_[2]; // rook queen
    uint32 n_minor_pieces_[2]; // bishop knight
    uint32 material_score_[2];

    uint32 cast_perm_;
    uint64 pos_key_;

    uint8 side_;
    uint8 fifty_move_counter_;

    uint32 ply_;
    TArray<FUndo> history_;

    bool is_multiplayer_;

public:
    TBoard();

    bool set(const FString& fen);

    bool make_move(const TMove& m);
    void take_move();

    FString ToString() const;

private:
    void reset();
    void update_material();
    uint64 generate_pos_key();

    bool is_valid();
    bool has_repetition();
    bool is_attacked(uint32 sq, uint8 attacking_side) const;

    void add_piece(uint32 sq, uint32 piece);
    void move_piece(uint32 from, uint32 to);
    void clear_piece(uint32 sq);
};
