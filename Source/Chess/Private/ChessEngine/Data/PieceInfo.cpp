// Copyright 2018 Emre Simsirli

#include "PieceInfo.h"
#include "Side.h"

const FPieceInfo piece_infos[] = {
    {0, ESide::both, false, false, false, false, {}},

    {100, ESide::white, false, false, false, false, {}},
    {325, ESide::white, true, false, false, false, {-8, -19, -21, -12, 8, 19, 21, 12}},
    {325, ESide::white, false, false, false, true, {-9, -11, 11, 9}},
    {550, ESide::white, false, false, true, false, {-1, -10, 1, 10}},
    {1000, ESide::white, false, false, true, true, {-1, -10, 1, 10, -9, -11, 11, 9}},
    {50000, ESide::white, false, true, false, false, {-1, -10, 1, 10, -9, -11, 11, 9}},

    {100, ESide::black, false, false, false, false, {}},
    {325, ESide::black, true, false, false, false, {-8, -19, -21, -12, 8, 19, 21, 12}},
    {325, ESide::black, false, false, false, true, {-9, -11, 11, 9}},
    {550, ESide::black, false, false, true, false, {-1, -10, 1, 10}},
    {1000, ESide::black, false, false, true, true, {-1, -10, 1, 10, -9, -11, 11, 9}},
    {50000, ESide::black, false, true, false, false, {-1, -10, 1, 10, -9, -11, 11, 9}}
};
