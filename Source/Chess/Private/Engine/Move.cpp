#include "Move.h"
#include <sstream>

namespace {
    const uint32 mask_move_from = 0x7F;
    const uint32 mask_move_to = 0x7F;
    const uint32 mask_move_captured = 0xF;
    const uint32 mask_move_is_ep = 0x1;
    const uint32 mask_move_pawn_start = 0x1;
    const uint32 mask_move_promoted_piece = 0x1;
    const uint32 mask_move_is_cast = 0x1;

    const uint32 shift_move_to = 7;
    const uint32 shift_move_captured = 14;
    const uint32 shift_move_is_ep = 18;
    const uint32 shift_move_pawn_start = 19;
    const uint32 shift_move_promoted_piece = 20;
    const uint32 shift_move_is_cast = 24;
}

engine::move::move_builder::move_builder() {
    m_ = 0;
}

engine::move::move_builder* engine::move::move_builder::from(const engine::square sq) {
    m_ |= static_cast<uint64>(sq);
    return this;
}

engine::move::move_builder* engine::move::move_builder::to(const engine::square sq) {
    m_ |= static_cast<uint64>(sq) << shift_move_to;
    return this;
}

engine::move::move_builder* engine::move::move_builder::captured_piece(const engine::piece_type piece) {
    m_ |= static_cast<uint64>(piece) << shift_move_captured;
    return this;
}

engine::move::move_builder* engine::move::move_builder::en_passant() {
    m_ |= 1ULL << shift_move_is_ep;
    return this;
}

engine::move::move_builder* engine::move::move_builder::pawn_start() {
    m_ |= 1ULL << shift_move_pawn_start;
    return this;
}

engine::move::move_builder* engine::move::move_builder::castling() {
    m_ |= 1ULL << shift_move_is_cast;
    return this;
}

engine::move engine::move::move_builder::build() {
    const uint64 m = m_;
    m_ = 0;
    return move(m);
}

engine::move::move(const uint32 m) {
    move_ = m;
}

uint32 engine::move::get(const uint32 shift, const uint32 and) const {
    return move_ >> shift & and;
}

uint32 engine::move::from() const {
    return move_ & mask_move_from;
}

uint32 engine::move::to() const {
    return get(shift_move_to, mask_move_to);
}

uint32 engine::move::captured_piece() const {
    return get(shift_move_captured, mask_move_captured);
}

uint32 engine::move::promoted_piece() const {
    return get(shift_move_promoted_piece, mask_move_promoted_piece);
}

bool engine::move::is_enpassant() const {
    return get(shift_move_is_ep, mask_move_is_ep);
}

bool engine::move::is_pawnstart() const {
    return get(shift_move_pawn_start, mask_move_pawn_start);
}

bool engine::move::is_castling() const {
    return get(shift_move_is_cast, mask_move_is_cast);
}

std::string engine::move::str() const {
    std::ostringstream stream;
    stream << "move - from: " << from() <<
        " to: " << to() <<
        " captured: " << captured_piece() <<
        " promoted: " << promoted_piece() <<
        " isep: " << is_enpassant() <<
        " ispawnstart: " << is_pawnstart() <<
        " iscast: " << is_castling() << '\n';
    return stream.str();
}
