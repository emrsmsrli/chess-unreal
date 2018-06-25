#include "Board.h"

engine::board::board() {}

engine::board::~board() = default;

engine::square engine::board::king_of(const side side) {
    return king_sq_[static_cast<int>(side)];
}
