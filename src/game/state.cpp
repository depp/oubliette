/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "entity.hpp"
#include "state.hpp"
#include "../opengl.hpp"
#include <cstdio>
#include <algorithm>
namespace game {

state::state()
    : initted_(false)
{
    set_level("main_wake");
}

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
    }
}

void state::draw(unsigned time)
{
    advance(time);
    int reltime = time - frametime_;
    graphics_.begin();
    graphics_.set_camera_pos(camera_.get_pos(reltime));
    for (auto i = entities_.begin(), e = entities_.end(); i != e; i++) {
        entity &ent = **i;
        ent.draw(graphics_, reltime);
    }
    graphics_.end();
    graphics_.draw();
}

void state::event_click(int x, int y, int button)
{
    if (button != 1 && button != 3)
        return;
    std::printf("click (%d, %d)\n", x, y);
}

void state::event_key(key k, bool state)
{
    control_.set_key(k, state);
}

void state::set_level(const std::string &name)
{
    add_entity(new player(*this, vec2(234, 46)));
    level_.set_level(name);
    camera_ = camera_system(
        rect(vec2::zero(), vec2(level_.width(), level_.height())));
    graphics_.set_level("main_wake");
}

void state::add_entity(std::unique_ptr<entity> &&ent)
{
    new_entities_.push_back(std::move(ent));
}

void state::add_entity(entity *ent)
{
    add_entity(std::unique_ptr<entity>(ent));
}

void state::set_camera_target(const rect &target)
{
    camera_.set_target(target);
}

}
