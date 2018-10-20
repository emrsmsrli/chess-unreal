// Copyright 2018 Emre Simsirli

#include "MoveGenerator.h"
#include "Verify.h"
#include "Piece.h"
#include "Board.h"
#include "Debug.h"
#include "BoardDefs.h"

using ::EPieceType;

namespace
{    
    const uint32 victim_score[] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
    int32 mvv_lva_scores[n_pieces][n_pieces];
}

void TMoveGenerator::Initialize()
{
    for(uint32 attacker = EPieceType::wp; attacker <= EPieceType::bk; attacker++)
        for(uint32 victim = EPieceType::wp; victim <= EPieceType::bk; victim++)
            mvv_lva_scores[victim][attacker] = victim_score[victim] + 6 - victim_score[attacker] / 100;
}

TMoveGenerator::TMoveGenerator(TBoard* ref)
{
    ref_ = ref;
}

TArray<TMove> TMoveGenerator::generate_moves() const
{
    MAKE_SURE(ref_->is_valid());

    uint32 piece_offset = 0;
    if(ref_->side_ == ESide::black)
        piece_offset = 6;

    TArray<TMove> moves;
    for(auto p = wp + piece_offset; p <= wk + piece_offset; ++p)
        for(auto sq : ref_->piece_locations_[p])
            moves.Append(generate_moves(sq));

    return moves;
}

TArray<TMove> TMoveGenerator::generate_moves(const uint32 sq) const
{
    const auto piece = ref_->b_[sq];
    const auto piece_info = pieces[piece];

    TArray<TMove> moves;
    if(piece_info.is_pawn) {
        generate_pawn_moves(sq, moves);
    } else if(piece_info.is_sliding) {
        generate_sliding_moves(sq, moves);
    } else {
        generate_non_sliding_moves(sq, moves);
        if(piece_info.is_king)
            generate_castling_moves(moves);
    }

    return moves;
}

bool TMoveGenerator::does_move_exist(const TMove& m) const
{
    // todo try optimising with generate_moves(m.from());
    const auto moves = generate_moves();
    for(auto& move : moves) {
        if(!ref_->make_move(move))
            continue;
        ref_->take_move();
        if(move == m)
            return true;
    }
    return false;
}

void TMoveGenerator::generate_pawn_moves(const uint32 sq, TArray<TMove>& moves) const
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));

    const auto d = ref_->side_ == ESide::white ? 1 : -1; // direction
    const auto other_side = ref_->side_ ^ 1;
    if(ref_->b_[sq + 10 * d] == empty) {
        add_pawn_regular_move(sq, sq + 10 * d, moves);
        if(ESquare::Rank(sq) == ERank::rank_2 && ref_->b_[sq + 20] == empty)
            add_quiet_move(TMove::create(sq, sq + 20, empty, empty, TMove::flag_pawn_start), moves);
        else if(ESquare::Rank(sq) == ERank::rank_7 && ref_->b_[sq - 20] == empty)
            add_quiet_move(TMove::create(sq, sq - 20, empty, empty, TMove::flag_pawn_start), moves);
    }

    if(Verification::IsSquareOnBoard(sq + 9 * d) && pieces[ref_->b_[sq + 9 * d]].side == other_side)
        add_pawn_capture_move(sq, sq + 9 * d, ref_->b_[sq + 9 * d], moves);
    if(Verification::IsSquareOnBoard(sq + 11 * d) && pieces[ref_->b_[sq + 11 * d]].side == other_side)
        add_pawn_capture_move(sq, sq + 11 * d, ref_->b_[sq + 11 * d], moves);

    if(ref_->en_passant_sq_ != ESquare::no_sq) {
        if(sq + 9 * d == ref_->en_passant_sq_)
            add_en_passant_move(TMove::create(sq, sq + 9 * d, empty, empty, TMove::flag_en_passant), moves);
        else if(sq + 11 * d == ref_->en_passant_sq_)
            add_en_passant_move(TMove::create(sq, sq + 11 * d, empty, empty, TMove::flag_en_passant), moves);
    }
}

void TMoveGenerator::generate_sliding_moves(const uint32 sq, TArray<TMove>& moves) const
{
    const auto piece = ref_->b_[sq];
    const auto other_side = ref_->side_ ^ 1;

    for(auto dir : pieces[piece].move_directions) {
        auto sqq = sq + dir;

        while(Verification::IsSquareOnBoard(sqq)) {
            if(ref_->b_[sqq] != empty) {
                if(pieces[ref_->b_[sqq]].side == other_side)
                    add_capture_move(TMove::create(sq, sqq, ref_->b_[sqq], empty, 0), moves);
                break;
            }
            add_quiet_move(TMove::create(sq, sqq, empty, empty, 0), moves);
            sqq += dir;
        }
    }
}

void TMoveGenerator::generate_non_sliding_moves(const uint32 sq, TArray<TMove>& moves) const
{
    const auto piece = ref_->b_[sq];
    const auto other_side = ref_->side_ ^ 1;
    for(auto dir : pieces[piece].move_directions) {
        const auto sqq = sq + dir;

        if(!Verification::IsSquareOnBoard(sqq))
            continue;

        if(ref_->b_[sqq] != empty) {
            if(pieces[ref_->b_[sqq]].side == other_side)
                add_capture_move(TMove::create(sq, sqq, ref_->b_[sqq], empty, 0), moves);
            continue;
        }
        add_quiet_move(TMove::create(sq, sqq, empty, empty, 0), moves);
    }
}

void TMoveGenerator::generate_castling_moves(TArray<TMove>& moves) const
{
    if(ref_->side_ == ESide::white) {
        if(ref_->cast_perm_ & ECastlingPermission::c_wk) {
            if(ref_->b_[ESquare::f1] == empty && ref_->b_[ESquare::g1] == empty) {
                if(!ref_->is_attacked(ESquare::e1, ESide::black) && !ref_->is_attacked(ESquare::f1, ESide::black)) {
                    add_quiet_move(TMove::create(ESquare::e1, ESquare::g1, empty, empty,
                                                 TMove::flag_castling), moves);
                }
            }
        }

        if(ref_->cast_perm_ & ECastlingPermission::c_wq) {
            if(ref_->b_[ESquare::d1] == empty
                && ref_->b_[ESquare::c1] == empty
                && ref_->b_[ESquare::b1] == empty) {
                if(!ref_->is_attacked(ESquare::e1, ESide::black) && !ref_->is_attacked(ESquare::d1, ESide::black)) {
                    add_quiet_move(TMove::create(ESquare::e1, ESquare::c1, empty, empty,
                                                 TMove::flag_castling), moves);
                }
            }
        }
    } else {
        if(ref_->cast_perm_ & ECastlingPermission::c_bk) {
            if(ref_->b_[ESquare::f8] == empty
                && ref_->b_[ESquare::g8] == empty) {
                if(!ref_->is_attacked(ESquare::e8, ESide::white) && !ref_->is_attacked(ESquare::f8, ESide::white)) {
                    add_quiet_move(TMove::create(ESquare::e8, ESquare::g8, empty, empty,
                                                 TMove::flag_castling), moves);
                }
            }
        }

        if(ref_->cast_perm_ & ECastlingPermission::c_bq) {
            if(ref_->b_[ESquare::d8] == empty
                && ref_->b_[ESquare::c8] == empty
                && ref_->b_[ESquare::b8] == empty) {
                if(!ref_->is_attacked(ESquare::e8, ESide::white) && !ref_->is_attacked(ESquare::d8, ESide::white)) {
                    add_quiet_move(TMove::create(ESquare::e8, ESquare::c8, empty, empty,
                                                 TMove::flag_castling), moves);
                }
            }
        }
    }
}

void TMoveGenerator::add_quiet_move(TMove move, TArray<TMove>& moves) const
{
    if(ref_->is_multiplayer_) {
        if(ref_->evaluator_.search_info_->killers[0][ref_->ply_] == move) {
           move.set_score(900000);
        } else if(ref_->evaluator_.search_info_->killers[1][ref_->ply_] == move) {
           move.set_score(800000);
        } else {
           move.set_score(ref_->evaluator_.search_info_->history[ref_->b_[move.from()]][move.to()]);
        }
    }
    moves.Add(move);
}

void TMoveGenerator::add_capture_move(TMove move, TArray<TMove>& moves) const
{
    if(ref_->is_multiplayer_)
        move.set_score(mvv_lva_scores[move.captured_piece()][ref_->b_[move.from()]] + 1000000);
    moves.Add(move);
}

void TMoveGenerator::add_en_passant_move(TMove move, TArray<TMove>& moves) const
{
    if(ref_->is_multiplayer_)
        move.set_score(105 + 1000000);
    moves.Add(move);
}

void TMoveGenerator::add_pawn_regular_move(const uint32 from, const uint32 to, TArray<TMove>& moves) const
{
    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));

    if(ref_->side_ == ESide::white && ESquare::Rank(from) == ERank::rank_7) {
        for(uint32 promoted : {wq, wr, wb, wn})
            add_quiet_move(TMove::create(from, to, empty, promoted, 0), moves);
    } else if(ref_->side_ == ESide::black && ESquare::Rank(from) == ERank::rank_2) {
        for(uint32 promoted : {bq, br, bb, bn})
            add_quiet_move(TMove::create(from, to, empty, promoted, 0), moves);
    } else {
        add_quiet_move(TMove::create(from, to, empty, empty, 0), moves);
    }
}

void TMoveGenerator::add_pawn_capture_move(const uint32 from, const uint32 to,
                                           const uint32 captured, TArray<TMove>& moves) const
{
    MAKE_SURE(Verification::IsPieceValidOrEmpty(captured));
    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));

    if(ref_->side_ == ESide::white && ESquare::Rank(from) == ERank::rank_7) {
        for(uint32 promoted : {wq, wr, wb, wn})
            add_capture_move(TMove::create(from, to, captured, promoted, 0), moves);
    } else if(ref_->side_ == ESide::black && ESquare::Rank(from) == ERank::rank_2) {
        for(uint32 promoted : {bq, br, bb, bn})
            add_capture_move(TMove::create(from, to, captured, promoted, 0), moves);
    } else {
        add_capture_move(TMove::create(from, to, captured, empty, 0), moves);
    }
}
