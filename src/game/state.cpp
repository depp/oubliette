/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "state.hpp"
#include "audio.hpp"
#include "defs.hpp"
#include "editor.hpp"
#include "entity.hpp"
#include "script.hpp"
#include "../defs.hpp"
#include <algorithm>
namespace game {

state::state(bool edit_mode)
    : edit_mode_(edit_mode), initted_(false)
{
    audio_.reset(new audio::system);
    if (!edit_mode)
        script_.reset(new script::script());
    persistent_.health = -1;
    persistent_.maxhealth = -1;
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
            if (scriptsys_->done())
                next_level();
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

void state::next_level()
{
    editor_.reset();
    entity_.reset();
    scriptsys_.reset();
    control_.clear();
    graphics_.clear_text();

    while (true) {
        if (levelqueue_.empty())
            core::die("No level");
        std::string next = levelqueue_.back();
        levelqueue_.pop_back();
        if (next.empty())
            core::die("Empty level name");

        if (next[0] == '@') {
            next = next.substr(1);
            auto sec = script_->get_section(next);
            if (sec == nullptr) {
                std::printf("Invalid section: %s\n", next.c_str());
                core::die("Could not load script");
            }
            scriptsys_.reset(new script::system(*sec, control_, *audio_));
            graphics_.set_level(std::string());
            return;
        } else if (next[0] == '!') {
            auto &st = persistent_;
            if (next == "!dead") {
                st.health = st.maxhealth;
                levelqueue_.push_back("main_wake");
                levelqueue_.push_back("@lose");
            } else if (next == "!health") {
                st.health = st.maxhealth;
            } else if (next == "!easy") {
                st.health = -1;
                st.maxhealth = -1;
                st.enemy_health = 1;
            } else if (next == "!normal") {
                st.health = 8;
                st.maxhealth = 8;
                st.enemy_health = 1;
            } else if (next == "!hard") {
                st.health = 3;
                st.maxhealth = 3;
                st.enemy_health = 2;
            } else if (next == "!end") {
                int treasure = st.treasure[0];
                for (int i = 1; i < 3; i++) {
                    if (st.treasure[i] != treasure)
                        treasure = 0;
                }
                if (treasure < 0 || treasure >= 5)
                    core::die("Invalid treasure");
                levelqueue_.push_back("difficulty");
                levelqueue_.push_back(
                    std::string("@epilogue_") + "01234"[treasure]);
                levelqueue_.push_back("@epilogue");
                st = persistent_state();
                levelname_.clear();
            } else if (next[1] == '!') {
                if (next.size() != 5)
                    core::die("Invalid cheat");
                for (int i = 0; i < 3; i++) {
                    int t = next[2+i] - '0';
                    if (t < 0 || t >= 5)
                        core::die("Invalid cheat treasure");
                    st.treasure[i] = t;
                }
            } else {
                core::die("Invalid command");
            }
        } else {
            std::string lastlevel(std::move(levelname_));
            levelname_ = next;
            entity_.reset(new entity_system(
                persistent_, control_, *audio_, next, lastlevel));
            entity_->update();
            graphics_.set_level(next);
            return;
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
    if (entity_)
        entity_->mouse_click(x, y, button);
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
    if (edit_mode_) {
        editor_.reset(new editor_system(control_, name));
        editor_->load_data();
        graphics_.set_level(name);
        return;
    }

    if (name.empty()) {
        std::puts("set_level: empty name");
        return;
    }

    levelqueue_.clear();
    std::size_t pos = 0;
    while (true) {
        std::size_t delim = name.find(':', pos);
        if (delim == std::string::npos) {
            std::string next = name.substr(pos);
            if (!next.empty())
                levelqueue_.push_back(std::move(next));
            break;
        } else {
            std::string next = name.substr(pos, delim - pos);
            if (!next.empty())
                levelqueue_.push_back(std::move(next));
            pos = delim + 1;
        }
    }

    std::reverse(levelqueue_.begin(), levelqueue_.end());
    next_level();
}

}
