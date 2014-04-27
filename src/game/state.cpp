/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "editor.hpp"
#include "entity.hpp"
#include "leveldata.hpp"
#include "state.hpp"
#include "../defs.hpp"
#include <algorithm>
namespace game {

state::state(bool edit_mode)
    : edit_mode_(edit_mode), initted_(false)
{ }

state::~state()
{ }

struct entity_is_dead {
    bool operator()(const std::unique_ptr<entity> &p) {
        return p->m_team != team::DEAD;
    }
};

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

    // std::printf("time = %u, nframes = %d\n", time, nframes);
    if (edit_mode_) {
        if (editor_ && nframes > 0) {
            editor_->update();
            control_.update();
        }
        return;
    }

    for (unsigned i = 0; i < nframes; i++) {
        auto part = std::stable_partition(
            entities_.begin(), entities_.end(), entity_is_dead());
        entities_.erase(part, entities_.end());
        entities_.insert(
            entities_.end(),
            std::make_move_iterator(new_entities_.begin()),
            std::make_move_iterator(new_entities_.end()));
        new_entities_.clear();
        for (auto i = entities_.begin(), e = entities_.end(); i != e; i++) {
            entity &ent = **i;
            ent.update();
        }
        camera_.update();
        control_.update();
    }
}

void state::draw(unsigned time)
{
    advance(time);
    int reltime = time - frametime_;
    graphics_.begin();

    if (edit_mode_) {
        if (editor_)
            editor_->draw(graphics_, reltime);
    } else {
        graphics_.set_camera_pos(camera_.get_pos(reltime));
        for (auto i = entities_.begin(), e = entities_.end(); i != e; i++) {
            entity &ent = **i;
            ent.draw(graphics_, reltime);
        }
    }

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
    levelname_ = name;

    entities_.clear();
    new_entities_.clear();
    control_.clear();

    if (edit_mode_) {
        editor_.reset(new editor_system(*this, name));
        editor_->load_data();
    } else {
        auto data = leveldata::read_level(name);
        if (data.empty())
            core::die("Could not load level");
        for (auto i = data.begin(), e = data.end(); i != e; i++)
            add_entity(entity::spawn(*this, *i));
    }

    level_.set_level(name);
    camera_ = camera_system(
        rect(vec2::zero(), vec2(level_.width(), level_.height())));
    graphics_.set_level(name);
}

void state::add_entity(std::unique_ptr<entity> &&ent)
{
    if (ent)
        new_entities_.push_back(std::move(ent));
}

void state::add_entity(entity *ent)
{
    if (ent)
        add_entity(std::unique_ptr<entity>(ent));
}

void state::set_camera_target(const rect &target)
{
    camera_.set_target(target);
}

}
