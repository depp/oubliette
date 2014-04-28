/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "state.hpp"
#include "defs.hpp"
#include "editor.hpp"
#include "entity.hpp"
#include "script.hpp"
#include "../defs.hpp"
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

    if (scriptsys_) {
        if (nframes > 0) {
            scriptsys_->update();
            control_.update();
        }
    } else if (entity_) {
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
    if (scriptsys_)
        scriptsys_->draw(graphics_, reltime);
    else if (entity_)
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
    if (name.empty()) {
        std::puts("set_level: empty name");
        return;
    }

    editor_.reset();
    entity_.reset();
    scriptsys_.reset();
    control_.clear();

    std::string target_script, target_level;
    auto pos = name.find(':');
    if (pos == std::string::npos) {
        target_level = name;
    } else {
        target_script = name.substr(0, pos);
        target_level = name.substr(pos + 1);
    }

    std::string lastlevel(std::move(levelname_));
    levelname_ = target_level;
    if (edit_mode_) {
        editor_.reset(new editor_system(control_, levelname_));
        editor_->load_data();
    } else {
        if (!target_script.empty()) {
            auto sec = script_->get_section(target_script);
            if (sec == nullptr) {
                std::printf("Invalid section: %s\n", target_script.c_str());
                core::die("Could not load script");
            }
            scriptsys_.reset(new script::system(*sec, control_));
        }
        if (!target_level.empty()) {
            entity_.reset(
                new entity_system(persistent_, control_,
                                  target_level, lastlevel));
        }
    }

    graphics_.set_level(target_level);
}

}
