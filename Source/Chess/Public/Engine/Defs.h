#pragma once

#include "CoreMinimal.h"

#define N_BOARD_SQUARES_X	120
#define N_BOARD_SQUARES		64
#define N_PIECES			13
//#define MAX_GAME_MOVES		2048

namespace engine {
    enum piece_type {
        empty, wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk
    };

    enum file {
        file_a, file_b, file_c, file_d, file_e, file_f, file_g, file_h, file_none
    };

    enum rank {
        rank_1, rank_2, rank_3, rank_4, rank_5, rank_6, rank_7, rank_8, rank_none
    };

    enum side {
        white, black, both
    };

    enum castling_permissions {
        c_wk = 1, c_wq = 2, c_bk = 4, c_bq = 8
    };

    enum square {
        a1 = 21, b1, c1, d1, e1, f1, g1, h1,
        a2 = 31, b2, c2, d2, e2, f2, g2, h2,
        a3 = 41, b3, c3, d3, e3, f3, g3, h3,
        a4 = 51, b4, c4, d4, e4, f4, g4, h4,
        a5 = 61, b5, c5, d5, e5, f5, g5, h5,
        a6 = 71, b6, c6, d6, e6, f6, g6, h6,
        a7 = 81, b7, c7, d7, e7, f7, g7, h7,
        a8 = 91, b8, c8, d8, e8, f8, g8, h8, no_sq, offboard
    };

    struct piece {
        bool is_big;
        bool is_major;
        bool is_minor;
        uint32 value;
        side side;

        piece(const bool ib, const bool imj, const bool imn, const uint32 v, const engine::side s)
            : is_big(ib), is_major(imj), is_minor(imn), value(v), side(s) {}
    };

    extern piece pieces[];

    namespace representation {
        extern char pieces[];
        extern char files[];
        extern char ranks[];
        extern char sides[];
	}
}
