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
        uint32 from() const;
        uint32 to() const;

        uint32 captured_piece() const;
        uint32 promoted_piece() const;

        bool is_enpassant() const;
        bool is_pawnstart() const;
        bool is_castling() const;

        uint32 score() const;

        static move create(uint32 score, engine::square from, engine::square to, 
							engine::piece_type captured, bool en_passant, 
							bool pawn_start, bool castling);

        std::string str() const;

    private:
        uint32 get(uint32 shift, uint32 and) const;
    };
}
