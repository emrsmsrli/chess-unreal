// Copyright 2018 Emre Simsirli

#pragma once

#include "ChessEngine/Board/Bitboard.h"
#include "ChessEngine/Move.h"
#include "ChessEngine/Definitions/Consts.h"
#include "ChessEngine/Undo.h"

constexpr auto max_depth = 64;

class TBoard {
    uint32 b_[n_board_squares_x];
    TBitboard pawns_[3];
    uint32 king_sq_[2];
    uint32 en_passant_sq_;

    uint32 n_big_pieces_[2];    // anything but pawn
    uint32 n_major_pieces_[2];  // rook queen
    uint32 n_minor_pieces_[2];  // bishop knight
    uint32 material_score_[2];

    uint32 cast_perm_;

    uint8 side_;
    uint32 fifty_move_counter_;

    uint32 current_search_ply_;
    TArray<FUndo> history_;

    TMove pv_array_[max_depth];
    TMap<uint64 /*pos_key*/, TMove> pv_table_;

    uint64 pos_key_;

    uint32 piece_count_[n_pieces];
    uint32 piece_list_[n_pieces][10];

public:
	TBoard();

    void reset();
    bool set(const FString& fen);
    uint64 generate_pos_key();
    void update_material();
    bool is_attacked(uint32 sq,uint32 side);

    TArray<TMove> generate_moves();
        
    bool make_move(const TMove& m);
    void take_move();
    bool move_exists(const TMove& m);

    void add_pv_move(const TMove& m);
    TMove probe_pv_table();
    uint32 get_pv_line(uint32 depth);

    bool is_valid();
    FString ToString() const;

    void perft(int32 depth, int64* leaf_nodes);
    FString perf_test(int32 depth);

private:
    bool has_repetition();

    void add_piece(uint32 sq, uint32 piece);
    void move_piece(uint32 from, uint32 to);
    void clear_piece(uint32 sq);

    static void add_white_pawn_capture_move(uint32 from, uint32 to, uint32 captured,
                                            TArray<TMove>& moves);
    static void add_white_pawn_move(uint32 from, uint32 to, TArray<TMove>& moves);
    static void add_black_pawn_capture_move(uint32 from, uint32 to, uint32 captured,
                                            TArray<TMove>& moves);
    static void add_black_pawn_move(uint32 from, uint32 to, TArray<TMove>& moves);

    static void add_quiet_move(TMove move, TArray<TMove>& moves);
    static void add_capture_move(TMove move, TArray<TMove>& moves);
    static void add_en_passant_move(TMove move, TArray<TMove>& moves);
};
