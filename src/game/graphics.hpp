/* Copyright 2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_GRAPHICS_HPP
#define LD_GAME_GRAPHICS_HPP

#include <memory>
namespace game {
class state;

class graphics_system {
private:
    struct data;

    std::unique_ptr<data> data_;

public:
    graphics_system();
    ~graphics_system();

    void init();
    void term();
    void draw(state &s, int reltime);
};

}

#endif
