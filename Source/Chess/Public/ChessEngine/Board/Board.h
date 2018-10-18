// Copyright 2018 Emre Simsirli

#pragma once

#include "Bitboard.h"
#include "Move.h"
#include "Consts.h"
#include "Undo.h"
#include "PrincipleVariation.h"
#include "MoveGenerator.h"

class TBoard
{
    friend class TMoveGenerator;
    friend class TPrincipleVariationTable;

    uint32 b_[n_board_squares_x];
    TBitboard pawns_[3];
    uint32 king_sq_[2];
    uint32 en_passant_sq_;

    uint32 n_big_pieces_[2]; // anything but pawn
    uint32 n_major_pieces_[2]; // rook queen
    uint32 n_minor_pieces_[2]; // bishop knight
    uint32 material_score_[2];

    uint32 cast_perm_;

    uint8 side_;
    uint32 fifty_move_counter_;

    uint32 ply_;
    TArray<FUndo> history_;

    TPrincipleVariationTable pv_table_ = TPrincipleVariationTable(this);
    TMoveGenerator move_generator_ = TMoveGenerator(this);

    uint64 pos_key_;

    TArray<uint32> piece_locations_[n_pieces];

public:
    TBoard();

    void reset();
    bool set(const FString& fen);
    uint64 generate_pos_key();
    void update_material();

    TArray<TMove> generate_moves();
    TArray<TMove> generate_moves(uint32 sq);

    bool make_move(const TMove& m);
    void take_move();
    bool move_exists(const TMove& m);

    bool is_attacked(uint32 sq, uint32 side) const;

    bool is_valid();
    FString ToString() const;

    void perft(int32 depth, int64* leaf_nodes);
    FString perf_test(int32 depth);

    int32 evaluate();
    void search(struct search_info& info);
    int32 alpha_beta(int32 alpha, int32 beta, uint32 depth, struct search_info& info, bool do_null);
    int32 quiescence(int32 alpha, int32 beta, struct search_info& info);

private:
    bool has_repetition();

    void add_piece(uint32 sq, uint32 piece);
    void move_piece(uint32 from, uint32 to);
    void clear_piece(uint32 sq);
};
