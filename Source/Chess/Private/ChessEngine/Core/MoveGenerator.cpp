// Copyright 2018 Emre Simsirli

#include "MoveGenerator.h"
#include "Verify.h"
#include "PieceInfo.h"
#include "Board.h"
#include "Debug.h"
#include "Search.h"
#include "ChessEngine.h"

using ::EPieceType;

namespace
{
    const uint32 victim_score[] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
    // most valuable victim - least valuable attacker
    int32 mvv_lva_scores[n_pieces][n_pieces];
}

void UMoveGenerator::Initialize()
{
    for(uint32 attacker = wp; attacker <= bk; attacker++)
        for(uint32 victim = wp; victim <= bk; victim++)
            mvv_lva_scores[victim][attacker] = victim_score[victim] + 6 - victim_score[attacker] / 100;
}

TArray<FMove> UMoveGenerator::GenerateMoves() const
{
    uint32 piece_offset = 0;
    if(CEngine->board_->side_ == ESide::black)
        piece_offset = 6;

    TArray<FMove> moves;
    for(auto p = wp + piece_offset; p <= wk + piece_offset; ++p)
        for(auto sq : CEngine->board_->piece_locations_[p])
            moves.Append(GenerateMoves(sq));

    return moves;
}

TArray<FMove> UMoveGenerator::GenerateMoves(const uint32 sq) const
{
    const auto piece = CEngine->board_->b_[sq];
    const auto piece_info = piece_infos[piece];

    TArray<FMove> moves;
    if(piece_info.bIsPawn) {
        GeneratePawnMoves(sq, moves);
    } else if(piece_info.bIsSliding) {
        GenerateSlidingMoves(sq, moves);
    } else {
        GenerateNonSlidingMoves(sq, moves);
        if(piece_info.bIsKing)
            GenerateCastlingMoves(moves);
    }

    return moves;
}

bool UMoveGenerator::DoesMoveExist(const FMove& m) const
{
    // todo try optimising with generate_moves(m.from());
    const auto moves = GenerateMoves();
    for(auto& move : moves) {
        if(!CEngine->board_->MakeMove(move))
            continue;
        CEngine->board_->TakeMove();
        if(move == m)
            return true;
    }
    return false;
}

void UMoveGenerator::GeneratePawnMoves(const uint32 sq, TArray<FMove>& moves) const
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));

    const auto d = CEngine->board_->side_ == ESide::white ? 1 : -1; // direction
    const auto other_side = CEngine->board_->side_ ^ 1;
    if(CEngine->board_->b_[sq + 10 * d] == empty) {
        AddPawnRegularMove(sq, sq + 10 * d, moves);
        if(ESquare::Rank(sq) == ERank::rank_2 && CEngine->board_->b_[sq + 20] == empty)
            AddQuietMove(FMove::Create(sq, sq + 20, empty, empty, FMove::flag_pawn_start), moves);
        else if(ESquare::Rank(sq) == ERank::rank_7 && CEngine->board_->b_[sq - 20] == empty)
            AddQuietMove(FMove::Create(sq, sq - 20, empty, empty, FMove::flag_pawn_start), moves);
    }

    if(Verification::IsSquareOnBoard(sq + 9 * d) && piece_infos[CEngine->board_->b_[sq + 9 * d]].Side == other_side)
        AddPawnCaptureMove(sq, sq + 9 * d, CEngine->board_->b_[sq + 9 * d], moves);
    if(Verification::IsSquareOnBoard(sq + 11 * d) && piece_infos[CEngine->board_->b_[sq + 11 * d]].Side == other_side)
        AddPawnCaptureMove(sq, sq + 11 * d, CEngine->board_->b_[sq + 11 * d], moves);

    if(CEngine->board_->en_passant_sq_ != ESquare::no_sq) {
        if(sq + 9 * d == CEngine->board_->en_passant_sq_)
            AddEnPassantMove(FMove::Create(sq, sq + 9 * d, empty, empty, FMove::flag_en_passant), moves);
        else if(sq + 11 * d == CEngine->board_->en_passant_sq_)
            AddEnPassantMove(FMove::Create(sq, sq + 11 * d, empty, empty, FMove::flag_en_passant), moves);
    }
}

void UMoveGenerator::GenerateSlidingMoves(const uint32 sq, TArray<FMove>& moves) const
{
    const auto piece = CEngine->board_->b_[sq];
    const auto other_side = CEngine->board_->side_ ^ 1;

    for(auto dir : piece_infos[piece].MoveDirections) {
        auto sqq = sq + dir;

        while(Verification::IsSquareOnBoard(sqq)) {
            if(CEngine->board_->b_[sqq] != empty) {
                if(piece_infos[CEngine->board_->b_[sqq]].Side == other_side)
                    AddCaptureMove(FMove::Create(sq, sqq, CEngine->board_->b_[sqq], empty, 0), moves);
                break;
            }
            AddQuietMove(FMove::Create(sq, sqq, empty, empty, 0), moves);
            sqq += dir;
        }
    }
}

void UMoveGenerator::GenerateNonSlidingMoves(const uint32 sq, TArray<FMove>& moves) const
{
    const auto piece = CEngine->board_->b_[sq];
    const auto other_side = CEngine->board_->side_ ^ 1;
    for(auto dir : piece_infos[piece].MoveDirections) {
        const auto sqq = sq + dir;

        if(!Verification::IsSquareOnBoard(sqq))
            continue;

        if(CEngine->board_->b_[sqq] != empty) {
            if(piece_infos[CEngine->board_->b_[sqq]].Side == other_side)
                AddCaptureMove(FMove::Create(sq, sqq, CEngine->board_->b_[sqq], empty, 0), moves);
            continue;
        }
        AddQuietMove(FMove::Create(sq, sqq, empty, empty, 0), moves);
    }
}

void UMoveGenerator::GenerateCastlingMoves(TArray<FMove>& moves) const
{
    if(CEngine->board_->side_ == ESide::white) {
        if(CEngine->board_->cast_perm_ & ECastlingPermission::c_wk) {
            if(CEngine->board_->b_[ESquare::f1] == empty && CEngine->board_->b_[ESquare::g1] == empty) {
                if(!CEngine->board_->IsAttacked(ESquare::e1, ESide::black) && !CEngine
                                                                               ->board_->IsAttacked(
                                                                                   ESquare::f1, ESide::black)) {
                    AddQuietMove(FMove::Create(ESquare::e1, ESquare::g1, empty, empty,
                                               FMove::flag_castling), moves);
                }
            }
        }

        if(CEngine->board_->cast_perm_ & ECastlingPermission::c_wq) {
            if(CEngine->board_->b_[ESquare::d1] == empty
                && CEngine->board_->b_[ESquare::c1] == empty
                && CEngine->board_->b_[ESquare::b1] == empty) {
                if(!CEngine->board_->IsAttacked(ESquare::e1, ESide::black) && !CEngine
                                                                               ->board_->IsAttacked(
                                                                                   ESquare::d1, ESide::black)) {
                    AddQuietMove(FMove::Create(ESquare::e1, ESquare::c1, empty, empty,
                                               FMove::flag_castling), moves);
                }
            }
        }
    } else {
        if(CEngine->board_->cast_perm_ & ECastlingPermission::c_bk) {
            if(CEngine->board_->b_[ESquare::f8] == empty
                && CEngine->board_->b_[ESquare::g8] == empty) {
                if(!CEngine->board_->IsAttacked(ESquare::e8, ESide::white) && !CEngine
                                                                               ->board_->IsAttacked(
                                                                                   ESquare::f8, ESide::white)) {
                    AddQuietMove(FMove::Create(ESquare::e8, ESquare::g8, empty, empty,
                                               FMove::flag_castling), moves);
                }
            }
        }

        if(CEngine->board_->cast_perm_ & ECastlingPermission::c_bq) {
            if(CEngine->board_->b_[ESquare::d8] == empty
                && CEngine->board_->b_[ESquare::c8] == empty
                && CEngine->board_->b_[ESquare::b8] == empty) {
                if(!CEngine->board_->IsAttacked(ESquare::e8, ESide::white) && !CEngine
                                                                               ->board_->IsAttacked(
                                                                                   ESquare::d8, ESide::white)) {
                    AddQuietMove(FMove::Create(ESquare::e8, ESquare::c8, empty, empty,
                                               FMove::flag_castling), moves);
                }
            }
        }
    }
}

void UMoveGenerator::AddQuietMove(FMove move, TArray<FMove>& moves) const
{
    if(CEngine->bIsMultiplayer) {
        if(CEngine->SearchInfo->GetKiller(0, CEngine->board_->ply_) == move) {
            move.SetScore(900000);
        } else if(CEngine->SearchInfo->GetKiller(1, CEngine->board_->ply_) == move) {
            move.SetScore(800000);
        } else {
            const auto score = CEngine->SearchInfo->GetHistory(CEngine->board_->b_[move.From()], move.To());
            move.SetScore(score);
        }
    }
    moves.Add(move);
}

void UMoveGenerator::AddCaptureMove(FMove move, TArray<FMove>& moves) const
{
    if(CEngine->bIsMultiplayer)
        move.SetScore(mvv_lva_scores[move.CapturedPiece()][CEngine->board_->b_[move.From()]] + 1000000);
    moves.Add(move);
}

void UMoveGenerator::AddEnPassantMove(FMove move, TArray<FMove>& moves) const
{
    if(CEngine->bIsMultiplayer)
        move.SetScore(105 + 1000000);
    moves.Add(move);
}

void UMoveGenerator::AddPawnRegularMove(const uint32 from, const uint32 to, TArray<FMove>& moves) const
{
    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));

    if(CEngine->board_->side_ == ESide::white && ESquare::Rank(from) == ERank::rank_7) {
        for(uint32 promoted : {wq, wr, wb, wn})
            AddQuietMove(FMove::Create(from, to, empty, promoted, 0), moves);
    } else if(CEngine->board_->side_ == ESide::black && ESquare::Rank(from) == ERank::rank_2) {
        for(uint32 promoted : {bq, br, bb, bn})
            AddQuietMove(FMove::Create(from, to, empty, promoted, 0), moves);
    } else {
        AddQuietMove(FMove::Create(from, to, empty, empty, 0), moves);
    }
}

void UMoveGenerator::AddPawnCaptureMove(const uint32 from, const uint32 to,
                                        const uint32 captured, TArray<FMove>& moves) const
{
    MAKE_SURE(Verification::IsPieceValidOrEmpty(captured));
    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));

    if(CEngine->board_->side_ == ESide::white && ESquare::Rank(from) == ERank::rank_7) {
        for(uint32 promoted : {wq, wr, wb, wn})
            AddCaptureMove(FMove::Create(from, to, captured, promoted, 0), moves);
    } else if(CEngine->board_->side_ == ESide::black && ESquare::Rank(from) == ERank::rank_2) {
        for(uint32 promoted : {bq, br, bb, bn})
            AddCaptureMove(FMove::Create(from, to, captured, promoted, 0), moves);
    } else {
        AddCaptureMove(FMove::Create(from, to, captured, empty, 0), moves);
    }
}
