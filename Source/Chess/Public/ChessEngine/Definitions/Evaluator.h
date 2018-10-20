// Copyright 2018 Emre Simsirli

#pragma once

#include "Platform.h"
#include "Function.h"

class TEvaluator
{
    friend class TMoveGenerator;

    class TBoard* ref_ = nullptr;
    class TPrincipleVariationTable* pv_table_= nullptr;
    struct search_info* search_info_ = nullptr;

public:
    explicit TEvaluator(TBoard* ref);

    void search(struct search_info& info, const TFunction<void(class TMove)>& callback) const;

private:
    int32 evaluate() const;
    int32 alpha_beta(int32 alpha, int32 beta, uint32 depth, struct search_info& info, bool do_null) const;
    int32 quiescence(int32 alpha, int32 beta, struct search_info& info) const;
};
