#pragma once

#include <string>
#include "CoreMinimal.h"
#include <functional>
#include "Defs.h"

namespace engine {
    class CHESS_API bitboard {
        const uint32 btable_[N_BOARD_SQUARES] = {
            63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13,
            11, 53, 19, 34, 61, 29, 2, 51, 21, 43, 45, 10,
            18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49,
            5, 52, 26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39,
            48, 24, 59, 14, 12, 55, 38, 28, 58, 20, 37, 17, 36, 8
        };

        uint64 board_;

    public:
        bitboard();
        
        void set_sq(uint32 sq);
        void clr_sq(uint32 sq);
        uint32 pop();
        uint32 count() const;
        bool is_empty() const;

        std::string str() const;
        uint64 operator*() const;
        bitboard &operator=(uint64 b);
    };

    namespace bitmask {
        void init();
    }
}
