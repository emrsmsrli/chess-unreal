#pragma once

#include <ostream>
#include "CoreMinimal.h"
#include "Defs.h"

namespace engine {
    class CHESS_API move {
        uint32 move_;
        uint32 score_;
        move(uint32 m, uint32 s);

    public:
        engine::square from() const;
        engine::square to() const;

        engine::piece_type captured_piece() const;
        engine::piece_type promoted_piece() const;

        bool is_captured() const;
        bool is_promoted() const;
        bool is_enpassant() const;
        bool is_pawnstart() const;
        bool is_castling() const;

        uint32 score() const;

        static move create(uint32 score, engine::square from, engine::square to, 
							engine::piece_type captured, bool en_passant, 
							bool pawn_start, engine::piece_type promoted, bool castling);

        std::string str() const;

    private:
        uint32 get(uint32 shift, uint32 and) const;
    };
}
