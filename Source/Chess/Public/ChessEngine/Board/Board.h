// Copyright 2018 Emre Simsirli

#pragma once

#include "Bitboard.h"
#include "Move.h"
#include "Consts.h"
#include "Undo.h"
#include "PrincipleVariation.h"

// todo move this....
void init_mvv_lva();

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

    uint64 pos_key_;

    TArray<uint32> piece_locations_[n_pieces];

    uint32 search_history_[n_pieces][n_board_squares];
    TMove search_killers_[2][max_depth];

public:
    TBoard();

    void reset();
    bool set(const FString& fen);
    uint64 generate_pos_key();
    void update_material();
    bool is_attacked(uint32 sq, uint32 attacking_side);

    TArray<TMove> generate_moves();

    bool make_move(const TMove& m);
    void take_move();
    bool move_exists(const TMove& m);

    bool is_valid();
    FString ToString() const;

    void perft(int32 depth, int64* leaf_nodes);
    FString perf_test(int32 depth);

    void clearforsearch(struct search_info& info);
    int32 evaluate();
    void search(struct search_info& info);
    int32 alpha_beta(int32 alpha, int32 beta, uint32 depth, struct search_info& info, bool do_null);
    int32 quiescence(int32 alpha, int32 beta, struct search_info& info);

private:
    bool has_repetition();

    void add_piece(uint32 sq, uint32 piece);
    void move_piece(uint32 from, uint32 to);
    void clear_piece(uint32 sq);

    void add_white_pawn_capture_move(uint32 from, uint32 to, uint32 captured,
                                     TArray<TMove>& moves);
    void add_white_pawn_move(uint32 from, uint32 to, TArray<TMove>& moves);
    void add_black_pawn_capture_move(uint32 from, uint32 to, uint32 captured,
                                     TArray<TMove>& moves);
    void add_black_pawn_move(uint32 from, uint32 to, TArray<TMove>& moves);

    void add_quiet_move(TMove move, TArray<TMove>& moves);
    void add_capture_move(TMove move, TArray<TMove>& moves);
    void add_en_passant_move(TMove move, TArray<TMove>& moves);
};
