#include "ChessEngine.h"
#include "Transition.h"
#include "Bitboard.h"
#include "PosKey.h"

void engine::init() {
    transition::init();
    bitmask::init();
	poskey::init();
}
