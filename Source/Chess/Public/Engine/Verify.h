#pragma once

// todo fix these
#define SQ_ON_BOARD(sq)         (engine::transition::sq_rank(sq) != static_cast<uint32>(engine::square::offboard))
#define SIDE_VALID(s)           (s == engine::side::white || s == engine::side::black)
#define FR_VALID(fr)            (fr >= 0 && fr <= 7)
#define PIECE_VALID_EMPTY(p)    (p >= engine::piece_type::empty && p <= engine::piece_type::bk)
#define PIECE_VALID(p)          (p >= engine::piece_type::wp && p <= engine::piece_type::bk)
