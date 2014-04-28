/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "state.hpp"
#include "defs.hpp"
#include "editor.hpp"
#include "entity.hpp"
#include "script.hpp"
namespace game {

state::state(bool edit_mode)
    : edit_mode_(edit_mode), initted_(false)
{
    if (!edit_mode)
        script_.reset(new script::script());
    persistent_.health = 2;
    persistent_.maxhealth = 5;
}

state::~state()
{ }

void state::advance(unsigned time)
{
    unsigned nframes;
    if (initted_) {
        unsigned delta = time - frametime_;
        if ((unsigned)delta > defs::MAXUPDATE) {
            std::puts("Lag!");
            nframes = 1;
            frametime_ = time;
        } else {
            nframes = delta / defs::FRAMETIME;
            frametime_ += nframes * defs::FRAMETIME;
        }
    } else {
        initted_ = true;
        frametime_ = time;
        nframes = 1;
    }

    if (entity_) {
        for (unsigned i = 0; i < nframes; i++) {
            entity_->update();
            control_.update();
            if (!entity_->nextlevel.empty()) {
                std::string level(std::move(entity_->nextlevel));
                set_level(level);
                break;
            }
        }
    } else if (editor_) {
        if (nframes > 0) {
            editor_->update();
            control_.update();
        }
    }
}

void state::draw(unsigned time)
{
    advance(time);
    int reltime = time - frametime_;
    graphics_.begin();
    if (entity_)
        entity_->draw(graphics_, reltime);
    else if (editor_)
        editor_->draw(graphics_, reltime);
    graphics_.end();
    graphics_.draw();
}

void state::mouse_click(int x, int y, int button)
{
    if (editor_)
        editor_->mouse_click(x, y, button);
}

void state::mouse_move(int x, int y)
{
    if (editor_)
        editor_->mouse_move(x, y);
}

void state::event_key(key k, bool state)
{
    control_.set_key(k, state);
}

void state::set_level(const std::string &name)
{
    std::string lastlevel(std::move(levelname_));
    levelname_ = name;
    control_.clear();

    if (edit_mode_) {
        editor_.reset(new editor_system(control_, name));
        editor_->load_data();
    } else {
        entity_.reset(
            new entity_system(persistent_, control_, name, lastlevel));
    }

    graphics_.set_level(name);
}

}
