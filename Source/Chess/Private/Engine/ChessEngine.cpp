#include "ChessEngine.h"
#include "Transition.h"
#include "Bitboard.h"
#include "PosKey.h"

namespace {
    std::string sf = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

void engine::init() {
    transition::init();
    bitmask::init();
	poskey::init();
}

std::string &engine::start_fen() {
    return sf;
}
