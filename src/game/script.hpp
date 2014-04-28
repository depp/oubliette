/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_SCRIPT_HPP
#define LD_GAME_SCRIPT_HPP
#include <string>
#include <unordered_map>
#include <vector>
namespace script {

enum class speaker { THERAPIST, PROTAGONIST, GAME };

struct line {
    speaker spkr;
    int lines;
    std::string text;
};

struct section {
    std::vector<line> lines;
};

class script {
private:
    std::unordered_map<std::string, section> sections_;

public:
    script();
    script(const script &) = delete;
    script(script &&) = delete;
    ~script();
    script &operator=(const script &) = delete;
    script &operator=(script &&) = delete;
};

}
#endif
