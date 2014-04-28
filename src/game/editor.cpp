/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "editor.hpp"
#include "control.hpp"
#include "leveldata.hpp"
#include "defs.hpp"
#include "graphics.hpp"
#include "../defs.hpp"
#include <algorithm>
namespace game {

static const int SEL_NONE = -1;
static const int SAVETIME = 15;

static const float EDITOR_CAMSPEED = 150.0f;

editor_system::editor_system(const control_system &control,
                             const std::string &levelname)
    : control_(control),
      levelname_(levelname),
      selection_(SEL_NONE),
      camera_pos_(core::PWIDTH / 2, core::PHEIGHT / 2),
      camera_lastpos_(camera_pos_),
      dragging_(false),
      panning_(false),
      type_(spawntype::PLAYER),
      dirty_(false),
      savetime_(0)
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
    return SEL_NONE;
}

void editor_system::window_to_world(int &x, int &y)
{
    auto cpos = camera_pos_ - vec2(core::PWIDTH / 2, core::PHEIGHT / 2);
    x += (int)std::round(cpos.x);
    y += (int)std::round(cpos.y);
}

void editor_system::mark_dirty()
{
    dirty_ = true;
    savetime_ = 0;
}

void editor_system::sort()
{
    auto b = entities_.begin(), e = entities_.end();
    if (selection_ >= 0) {
        for (auto i = b; i != e; i++)
            i->flag = false;
        entities_[selection_].flag = true;
        std::stable_sort(b, e);
        selection_ = SEL_NONE;
        for (auto i = b; i != e; i++) {
            if (i->flag) {
                selection_ = i - b;
                break;
            }
        }
    } else {
        std::stable_sort(b, e);
    }
}

void editor_system::update()
{
    if (dirty_ && !dragging_) {
        savetime_++;
        if (savetime_ >= SAVETIME)
            save_data();
    }
    camera_lastpos_ = camera_pos_;
    float speed = EDITOR_CAMSPEED * (1e-3 * defs::FRAMETIME);
    camera_pos_ += vec2(control_.get_xaxis(), control_.get_yaxis()) * speed;
}

void editor_system::draw(::graphics::system &gr, int reltime)
{
    if (selection_ >= 0)
        gr.set_selection(entities_.at(selection_).bounds().expand(2));
    gr.set_camera_pos(defs::interp(camera_lastpos_, camera_pos_, reltime));
    for (auto i = entities_.begin(), e = entities_.end(); i != e; i++)
        i->draw(gr);
}

void editor_system::load_data()
{
    entities_ = leveldata::read_level(levelname_);
    selection_ = SEL_NONE;
    dragging_ = false;
    panning_ = false;
    dirty_ = true;
    sort();
}

void editor_system::save_data()
{
    std::printf("Saving level %s\n", levelname_.c_str());
    leveldata::write_level(levelname_, entities_);
    dirty_ = false;
}

void editor_system::mouse_click(int x, int y, int button)
{
    window_to_world(x, y);
    dragging_ = false;
    panning_ = false;
    switch (button) {
    case -1:
        break;

    case 1:
        selection_ = hit(x, y);
        if (selection_ >= 0) {
            auto &s = entities_.at(selection_);
            dragging_ = true;
            clickx_ = s.x - x;
            clicky_ = s.y - y;
            dragging_ = true;
        }
        break;

    case 2:
        clickx_ = x;
        clicky_ = y;
        panning_ = true;
        break;

    case 3:
        mark_dirty();
        clickx_ = 0;
        clicky_ = 0;
        selection_ = entities_.size();
        dragging_ = true;
        spawnpoint pt;
        pt.x = x;
        pt.y = y;
        pt.type = type_;
        entities_.push_back(std::move(pt));
        sort();
        break;
    }
}

void editor_system::mouse_move(int x, int y)
{
    if (dragging_) {
        window_to_world(x, y);
        auto &s = entities_.at(selection_);
        s.x = x + clickx_;
        s.y = y + clicky_;
        mark_dirty();
    } else if (panning_) {
        camera_pos_.x = clickx_ - x + core::PWIDTH / 2;
        camera_pos_.y = clicky_ - y + core::PHEIGHT / 2;
        camera_lastpos_ = camera_pos_;
    }
}

}
