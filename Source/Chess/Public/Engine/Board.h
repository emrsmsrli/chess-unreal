#pragma once

#include <vector>
#include "Bitboard.h"
#include "Undo.h"
#include "CoreMinimal.h"
#include "Defs.h"

namespace engine {
    class CHESS_API board {
        uint32 b_[N_BOARD_SQUARES_X];
        bitboard pawns_[3];
        square king_sq_[2];
        square en_passant_sq_;

        uint32 n_big_pieces_[2];    // anything but pawn
        uint32 n_major_pieces_[2];  // rook queen
        uint32 n_minor_pieces_[2];  // bishop knight
        uint32 material_score_[2];

        uint32 cast_perm_;

        side side_;
        uint32 fifty_move_counter_;

        uint32 current_search_ply_;
        std::vector<undo> history_;

        uint64 pos_key_;

        uint32 piece_count_[N_PIECES];
        square piece_list_[N_PIECES][10];

    public:
        board();

        void reset();
        bool set(const std::string& fen);
        uint64 generate_pos_key();
        void update_material();

        bool is_attacked(square sq, side side);
        std::vector<engine::move>* generate_moves();

        bool is_valid();
        std::string str() const;

    private:
        void add_piece(square sq, piece_type piece);
        void move_piece(square from, square to);
        void clear_piece(square sq);

        static void add_white_pawn_capture_move(square from, square to, piece_type captured,
                                                std::vector<engine::move>* moves);
        static void add_white_pawn_move(square from, square to, std::vector<engine::move>* moves);
        static void add_black_pawn_capture_move(square from, square to, piece_type captured,
                                                std::vector<engine::move>* moves);
        static void add_black_pawn_move(square from, square to, std::vector<engine::move>* moves);

        static void add_quiet_move(move* move, std::vector<engine::move>* moves);
        static void add_capture_move(move* move, std::vector<engine::move>* moves);
        static void add_en_passant_move(move* move, std::vector<engine::move>* moves);
    };
}
