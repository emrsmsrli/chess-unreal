// Copyright 2018 Emre Simsirli

#pragma once

enum ECastlingPermission
{
    c_wk = 1, c_wq = 2, c_bk = 4, c_bq = 8
};

struct search_info
{
    int starttime;
    int stoptime;
    int depth;
    int timeset;
    int movestogo;

    long nodes;

    int quit;
    int stopped;

    float fh; // fail high
    float fhf; // fail high first
    int nullCut;

    int GAME_MODE;
    int POST_THINKING;
};
