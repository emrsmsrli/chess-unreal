#pragma once

#include <ostream>
#include "CoreMinimal.h"
#include "Defs.h"

namespace engine {
    class CHESS_API move {
    public:
        static const uint32 flag_en_passant = 0x40000;
        static const uint32 flag_pawn_start = 0x80000;
        static const uint32 flag_castling = 0x1000000;

        static const move no_move;

    private:
        uint32 move_;
        uint32 score_ = 0;
        explicit move(uint32 m);

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

        void set_score(uint32 s);
        uint32 score() const;

        static move create(engine::square from, engine::square to,
                           engine::piece_type captured,
                           engine::piece_type promoted, uint32 flags);

        std::string str() const;

    private:
        uint32 get(uint32 shift, uint32 and) const;
    };
}
