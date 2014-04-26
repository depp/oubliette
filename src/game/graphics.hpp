/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_GRAPHICS_HPP
#define LD_GAME_GRAPHICS_HPP
#include <memory>
namespace image {
struct texture;
}
namespace game {
class state;

class graphics_system {
private:
    struct data;

    std::unique_ptr<data> data_;
    data &getdata();

public:
    graphics_system();
    ~graphics_system();

    void init();
    void term();
    void draw(state &s, int reltime);

    void set_level(const std::string &path);
};

}
#endif
