#pragma once

#include <string>

// todo move these to another file. this file should only contain engine interface functions
namespace engine {
    void init();
    std::string &start_fen();
}
