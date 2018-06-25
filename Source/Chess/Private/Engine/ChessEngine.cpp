#include "ChessEngine.h"
#include "Transition.h"
#include "Bitboard.h"

void engine::init() {
    transition::init();
    bitmask::init();
}
