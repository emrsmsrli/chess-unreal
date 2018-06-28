#include "Move.h"
#include "Transition.h"
#include <sstream>

namespace {
    const uint32 mask_move_from = 0x7F;
    const uint32 mask_move_to = 0x7F;
    const uint32 mask_move_captured = 0xF;
    const uint32 mask_move_promoted_piece = 0xF;

    const uint32 mask_move_is_capture = 0x7C000;
    const uint32 mask_move_is_promote = 0xF00000;

    const uint32 shift_move_to = 7;
    const uint32 shift_move_captured = 14;
    const uint32 shift_move_promoted_piece = 20;
}

engine::move::move(const uint32 m) {
    move_ = m;
}

uint32 engine::move::get(const uint32 shift, const uint32 and) const {
    return move_ >> shift & and;
}

engine::square engine::move::from() const {
    return static_cast<engine::square>(move_ & mask_move_from);
}

engine::square engine::move::to() const {
    return static_cast<engine::square>(get(shift_move_to, mask_move_to));
}

engine::piece_type engine::move::captured_piece() const {
    return static_cast<engine::piece_type>(get(shift_move_captured, mask_move_captured));
}

engine::piece_type engine::move::promoted_piece() const {
    return static_cast<engine::piece_type>(get(shift_move_promoted_piece, mask_move_promoted_piece));
}

bool engine::move::is_captured() const {
    return (move_ & mask_move_is_capture) != 0;
}

bool engine::move::is_promoted() const {
    return (move_ & mask_move_is_promote) != 0;
}

bool engine::move::is_enpassant() const {
    return (move_ & flag_en_passant) != 0;
}

bool engine::move::is_pawnstart() const {
    return (move_ & flag_pawn_start) != 0;
}

bool engine::move::is_castling() const {
    return (move_ & flag_castling) != 0;
}

void engine::move::set_score(const uint32 s) {
    score_ = s;
}

uint32 engine::move::score() const {
    return score_;
}

engine::move engine::move::create(const engine::square from, const engine::square to,
                                  const engine::piece_type captured,
                                  const engine::piece_type promoted, const uint32 flags = 0) {
    uint32 m = 0;
    m |= from;
    m |= to << shift_move_to;
    m |= captured << shift_move_captured;
    m |= promoted << shift_move_promoted_piece;
    m |= flags;
    return {m};
}

std::string engine::move::str() const {
    std::ostringstream stream;
    stream << representation::sq_str(from()) << representation::sq_str(to());

    if(is_promoted()) {
        const auto p = pieces[promoted_piece()];
        if(p.is_knight) {
            stream.put('n');
        } else if(p.is_rook_queen && !p.is_bishop_queen) {
            stream.put('r');
        } else if(!p.is_rook_queen && p.is_bishop_queen) {
            stream.put('b');
        } else {
            stream.put('q');
        }
    }

    return stream.str();
}
