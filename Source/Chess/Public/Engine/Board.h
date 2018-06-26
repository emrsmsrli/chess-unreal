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

        uint32 piece_count_[N_PIECES];
        uint32 n_big_pieces_[3];    // anything but pawn
        uint32 n_major_pieces_[3];  // rook queen
        uint32 n_minor_pieces_[3];  // bishop knight

        uint32 cast_perm_;

        side side_;
        uint32 fifty_move_counter_;

        uint32 current_search_ply_;
        std::vector<undo> history_;

        uint64 pos_key_;

        square piece_list_[N_PIECES][10];

    public:
        board();

        void reset();
        bool set(const std::string &fen);

        uint64 generate_pos_key();
        square king_of(side side);

        std::string str() const;
    };
}
