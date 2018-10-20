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

    uint32 history[n_pieces][n_board_squares];
    TMove killers[2][max_depth];

    search_info();
    void add_killer(uint32 ply, TMove& move);

    void reset();
};

FORCEINLINE search_info::search_info()
{
    reset();
}

FORCEINLINE void search_info::add_killer(const uint32 ply, TMove& move)
{
    killers[1][ply] = killers[0][ply];
    killers[0][ply] = move;
}

FORCEINLINE void search_info::reset()
{
    starttime = 0;
    stoptime = 0;
    depth = 0;
    timeset = 0;
    movestogo = 0;

    nodes = 0;

    quit = 0;
    stopped = 0;

    fh = 0;
    fhf = 0;
    nullCut = 0;

    GAME_MODE = 0;
    POST_THINKING = 0;

    for(auto& i : history) {
        for(auto& j : i) {
            j = 0;
        }
    }

    for(auto& i : killers) {
        for(auto& j : i) {
            j = TMove::no_move;
        }
    }
}
