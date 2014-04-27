/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "editor.hpp"
#include "leveldata.hpp"
#include "defs.hpp"
#include "state.hpp"
#include "../defs.hpp"
namespace game {

static const float EDITOR_CAMSPEED = 150.0f;

editor_system::editor_system(const state &st)
    : state_(st),
      selection_(-1),
      camera_pos_(core::PWIDTH / 2, core::PHEIGHT / 2),
      camera_lastpos_(camera_pos_),
      dragging_(false),
      type_(spawntype::PLAYER)
{ }

editor_system::~editor_system()
{ }

int editor_system::hit(int x, int y)
{
    auto b = entities_.begin(), e = entities_.end();
    for (auto i = b; i != e; i++) {
        auto bbox = i->bounds();
        if (x >= bbox.x0 && x < bbox.x1 &&
            y >= bbox.y0 && y < bbox.y1)
            return i - b;
    }
    return -1;
}

void editor_system::window_to_world(int &x, int &y)
{
    auto cpos = camera_pos_ - vec2(core::PWIDTH / 2, core::PHEIGHT / 2);
    x += (int)std::round(cpos.x);
    y += (int)std::round(cpos.y);
}

void editor_system::update()
{
    camera_lastpos_ = camera_pos_;
    float speed = EDITOR_CAMSPEED * (1e-3 * defs::FRAMETIME);
    auto &c = state_.control();
    camera_pos_ += vec2(c.get_xaxis(), c.get_yaxis()) * speed;
}

void editor_system::draw(::graphics::system &gr, int reltime)
{
    gr.set_camera_pos(defs::interp(camera_lastpos_, camera_pos_, reltime));
    for (auto i = entities_.begin(), e = entities_.end(); i != e; i++)
        i->draw(gr);
}

void editor_system::load_data(std::vector<spawnpoint> &&data)
{
    entities_ = std::move(data);
    selection_ = -1;
    dragging_ = false;
}

void editor_system::save_data(const std::string &levelname)
{
    leveldata::write_level(levelname, entities_);
}

void editor_system::mouse_click(int x, int y, int button)
{
    window_to_world(x, y);
    switch (button) {
    case -1:
        dragging_ = false;
        break;

    case 1:
        selection_ = hit(x, y);
        if (selection_ >= 0) {
            auto &s = entities_.at(selection_);
            dragging_ = true;
            clickx_ = s.x - x;
            clicky_ = s.y - y;
        } else {
            dragging_ = false;
        }
        break;

    case 3:
        clickx_ = 0;
        clicky_ = 0;
        selection_ = entities_.size();
        dragging_ = true;
        spawnpoint pt;
        pt.x = x;
        pt.y = y;
        pt.type = type_;
        entities_.push_back(std::move(pt));
        break;
    }
}

void editor_system::mouse_move(int x, int y)
{
    window_to_world(x, y);
    if (dragging_ && selection_ >= 0) {
        auto &s = entities_.at(selection_);
        s.x = x + clickx_;
        s.y = y + clicky_;
    }
}

}
