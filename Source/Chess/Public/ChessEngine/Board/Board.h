// Copyright 2018 Emre Simsirli

#pragma once

#include "Debug.h"
#include "Object.h"
#include "Bitboard.h"
#include "Move.h"
#include "Consts.h"
#include "Undo.h"
#include "PrincipleVariation.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "Board.generated.h"

class UMoveGenerator;
class UPrincipleVariationTable;
class UEvaluator;

UCLASS()
class CHESS_API UBoard : public UObject
{
    GENERATED_BODY()

    friend UMoveGenerator;
    friend UPrincipleVariationTable;
    friend UEvaluator;

    uint32 b_[n_board_squares_x];
    FBitboard pawns_[3];
    uint32 king_sq_[2];
    uint32 en_passant_sq_;

    TArray<uint32> piece_locations_[n_pieces];

    uint32 n_big_pieces_[2]; // anything but pawn
    uint32 n_major_pieces_[2]; // rook queen
    uint32 n_minor_pieces_[2]; // bishop knight
    uint32 material_score_[2];

    uint32 cast_perm_;
    uint64 pos_key_;

    uint8 side_;
    uint8 fifty_move_counter_;

    uint32 ply_;
    TArray<FUndo> history_;

public:
    UBoard();

    bool Set(const FString& fen);

    bool MakeMove(const FMove& m);
    void TakeMove();

private:
    void Reset();
    void UpdateMaterial();
    uint64 GeneratePositionKey() const;

    bool HasRepetition();
    bool IsAttacked(uint32 sq, uint8 attacking_side) const;

    void AddPiece(uint32 sq, uint32 piece);
    void MovePiece(uint32 from, uint32 to);
    void ClearPiece(uint32 sq);

#ifdef DEBUG
public:
    FString ToString() const;
private:
    bool IsOk() const;
#endif
};
