#pragma once

#include <ostream>
#include "CoreMinimal.h"
#include "Defs.h"

namespace engine {
    class CHESS_API move {
        class CHESS_API move_builder {
        public:
            move_builder();

        private:
            uint32 m_;

        public:
            move_builder* from(engine::square sq);
            move_builder* to(engine::square sq);
            move_builder* captured_piece(engine::piece_type piece);
            move_builder* en_passant();
            move_builder* pawn_start();
            move_builder* castling();
            move build();
        };

        uint32 move_;
        explicit move(uint32 m);

    public:
        uint32 from() const;
        uint32 to() const;

        uint32 captured_piece() const;
        uint32 promoted_piece() const;

        bool is_enpassant() const;
        bool is_pawnstart() const;
        bool is_castling() const;

        static move_builder* builder() {
            static move_builder builder;
            return &builder;
        }

        // todo fix these functions returning ints instead of strs, use macros if possible.
        friend std::ostream& operator<<(std::ostream& stream, move& m) {
            stream << "move - from: " << m.from() <<
                " to: " << m.to() <<
                " captured: " << m.captured_piece() <<
                " promoted: " << m.promoted_piece() <<
                " isep: " << m.is_enpassant() <<
                " ispawnstart: " << m.is_pawnstart() <<
                " iscast: " << m.is_castling() << '\n';
            return stream;
        }

    private:
        uint32 get(uint32 shift, uint32 and) const;
    };
}
