#include "Bitboard.h"
#include <sstream>
#include "ChessEngine.h"
#include "Transition.h"

engine::bitboard::bitboard() {
    board_ = 0;
}

void engine::bitboard::set_sq(const uint32 sq) {
    board_ |= shift_ << sq;
}

uint32 engine::bitboard::pop() {
    const uint64 b = board_ ^ (board_ - 1);
    const uint32 fold = (b & 0xFFFFFFFF) ^ (b >> 32);
    board_ &= board_ - 1;
    return btable_[fold * 0x783a9b23 >> 26];
}

uint32 engine::bitboard::count() const {
    auto board = board_;
    uint32 i = 0;
    for(; board; i++, board &= board - 1)
		;
    return i;
}

std::string engine::bitboard::str() const {
    std::ostringstream stream;
    const uint64 s = 1L;

    for(uint32 rank = rank::rank_8; rank >= rank::rank_1; --rank) {
        for(uint32 file = file::file_a; file <= file::file_h; ++file) {
            const auto sq = transition::file_rank_sq120(file, rank);
            const auto sq64 = transition::sq64(sq);

            if(s << sq64 & board_)
                stream << "X";
            else
                stream << "-";
        }
        stream << "\n";
    }

    return stream.str();
}
