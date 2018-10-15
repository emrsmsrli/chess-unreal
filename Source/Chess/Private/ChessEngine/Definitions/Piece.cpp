// Copyright 2018 Emre Simsirli

#include "Piece.h"
#include "Side.h"

TPiece pieces[] = {
    {0, ESide::both, false, false, false, false},

    {100, ESide::white, false, false, false, false},
    {325, ESide::white, true, false, false, false},
    {325, ESide::white, false, false, false, true},
    {550, ESide::white, false, false, true, false},
    {1000, ESide::white, false, false, true, true},
    {50000, ESide::white, false, true, false, false},

    {100, ESide::black, false, false, false, false},
    {325, ESide::black, true, false, false, false},
    {325, ESide::black, false, false, false, true},
    {550, ESide::black, false, false, true, false},
    {1000, ESide::black, false, false, true, true},
    {50000, ESide::black, false, true, false, false}
};
