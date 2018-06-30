#pragma once

#include <string>
#include "Move.h"

namespace engine {
    move parse_move(const std::string& mstr);

    void init();
}
