// Copyright 2018 Emre Simsirli

#include "Move.h"
#include "UnrealString.h"
#include "PieceInfo.h"
#include "Square.h"

namespace
{
	const uint32 mask_move_from = 0x7F;
	const uint32 mask_move_to = 0x7F;
	const uint32 mask_move_captured = 0xF;
	const uint32 mask_move_promoted_piece = 0xF;

	const uint32 mask_move_is_capture = 0x7C000;
	const uint32 mask_move_is_promote = 0xF00000;

	const uint32 shift_move_to = 7;
	const uint32 shift_move_captured = 14;
	const uint32 shift_move_promoted_piece = 20;
}

const FMove FMove::no_move
	= FMove::create(ESquare::no_sq, ESquare::no_sq, EPieceType::empty, EPieceType::empty, 0);

FMove::FMove(const uint32 m)
{
	move_ = m;
}

uint32 FMove::get(const uint32 shift, const uint32 mask) const
{
	return move_ >> shift & mask;
}

uint32 FMove::from() const
{
	return move_ & mask_move_from;
}

uint32 FMove::to() const
{
	return get(shift_move_to, mask_move_to);
}

uint32 FMove::captured_piece() const
{
	return get(shift_move_captured, mask_move_captured);
}

uint32 FMove::promoted_piece() const
{
	return get(shift_move_promoted_piece, mask_move_promoted_piece);
}

bool FMove::is_captured() const
{
	return (move_ & mask_move_is_capture) != 0;
}

bool FMove::is_promoted() const
{
	return (move_ & mask_move_is_promote) != 0;
}

bool FMove::is_enpassant() const
{
	return (move_ & flag_en_passant) != 0;
}

bool FMove::is_pawnstart() const
{
	return (move_ & flag_pawn_start) != 0;
}

bool FMove::is_castling() const
{
	return (move_ & flag_castling) != 0;
}

void FMove::set_score(const uint32 s)
{
	score_ = s;
}

uint32 FMove::score() const
{
	return score_;
}

FMove FMove::create(const uint32 from, const uint32 to, const uint32 captured,
                    const uint32 promoted, const uint32 flags = 0)
{
	uint32 m = 0;
	m |= from;
	m |= to << shift_move_to;
	m |= captured << shift_move_captured;
	m |= promoted << shift_move_promoted_piece;
	m |= flags;
	return FMove(m);
}

FString FMove::ToString() const
{
	auto str = ESquare::AsString(from()) + ESquare::AsString(to());
	if (is_promoted()) {
		const auto piece_info = piece_infos[promoted_piece()];
		if (piece_info.is_knight)
			str += 'n';
		else if (piece_info.is_rook_queen && !piece_info.is_bishop_queen)
			str += 'r';
		else if (!piece_info.is_rook_queen && piece_info.is_bishop_queen)
			str += 'b';
		else
			str += 'q';
	}

	return str;
}

bool FMove::operator==(const FMove& o) const
{
	return move_ == o.move_;
}

bool FMove::operator!=(const FMove& o) const
{
	return !FMove::operator==(o);
}
