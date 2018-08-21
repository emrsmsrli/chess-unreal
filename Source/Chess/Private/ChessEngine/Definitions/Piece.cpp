// Copyright 2018 Emre Simsirli

#include "ChessEngine/Definitions/Piece.h"
#include "ChessEngine/Definitions/Team.h"

TPiece pieces[] = {
    {0, ETeam::BOTH, false, false, false, false},

    {100, ETeam::WHITE, false, false, false, false},
    {325, ETeam::WHITE, true, false, false, false},
    {325, ETeam::WHITE, false, false, false, true},
    {550, ETeam::WHITE, false, false, true, false},
    {1000, ETeam::WHITE, false, false, true, true},
    {50000, ETeam::WHITE, false, true, false, false},

    {100, ETeam::BLACK, false, false, false, false},
    {325, ETeam::BLACK, true, false, false, false},
    {325, ETeam::BLACK, false, false, false, true},
    {550, ETeam::BLACK, false, false, true, false},
    {1000, ETeam::BLACK, false, false, true, true},
    {50000, ETeam::BLACK, false, true, false, false}
};
