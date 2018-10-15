// Copyright 2018 Emre Simsirli

#include "Move.h"
#include "Piece.h"
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

const TMove TMove::no_move
	= TMove::create(ESquare::no_sq, ESquare::no_sq, EPieceType::empty, EPieceType::empty, 0);

TMove::TMove(const uint32 m)
{
	move_ = m;
}

uint32 TMove::get(const uint32 shift, const uint32 mask) const
{
	return move_ >> shift & mask;
}

uint32 TMove::from() const
{
	return move_ & mask_move_from;
}

uint32 TMove::to() const
{
	return get(shift_move_to, mask_move_to);
}

uint32 TMove::captured_piece() const
{
	return get(shift_move_captured, mask_move_captured);
}

uint32 TMove::promoted_piece() const
{
	return get(shift_move_promoted_piece, mask_move_promoted_piece);
}

bool TMove::is_captured() const
{
	return (move_ & mask_move_is_capture) != 0;
}

bool TMove::is_promoted() const
{
	return (move_ & mask_move_is_promote) != 0;
}

bool TMove::is_enpassant() const
{
	return (move_ & flag_en_passant) != 0;
}

bool TMove::is_pawnstart() const
{
	return (move_ & flag_pawn_start) != 0;
}

bool TMove::is_castling() const
{
	return (move_ & flag_castling) != 0;
}

void TMove::set_score(const uint32 s)
{
	score_ = s;
}

uint32 TMove::score() const
{
	return score_;
}

TMove TMove::create(const uint32 from, const uint32 to, const uint32 captured,
                    const uint32 promoted, const uint32 flags = 0)
{
	uint32 m = 0;
	m |= from;
	m |= to << shift_move_to;
	m |= captured << shift_move_captured;
	m |= promoted << shift_move_promoted_piece;
	m |= flags;
	return TMove(m);
}

FString TMove::ToString() const
{
	auto str = ESquare::AsString(from()) + ESquare::AsString(to());
	if (is_promoted()) {
		const auto p = pieces[promoted_piece()];
		if (p.is_knight)
			str += 'n';
		else if (p.is_rook_queen && !p.is_bishop_queen)
			str += 'r';
		else if (!p.is_rook_queen && p.is_bishop_queen)
			str += 'b';
		else
			str += 'q';
	}

	return str;
}

bool TMove::operator==(const TMove& o) const
{
	return move_ == o.move_;
}

bool TMove::operator!=(const TMove& o) const
{
	return !TMove::operator==(o);
}
