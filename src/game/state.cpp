/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "entity.hpp"
#include "state.hpp"
#include "../defs.hpp"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <errno.h>
namespace game {

namespace spawninfo {

static const char SPAWN_TYPES[][8] = {
    "player",
    "door",
    "chest",
    "slime",
    "prof",
    "woman",
    "priest"
};

static const int NTYPE = sizeof(SPAWN_TYPES) / sizeof(*SPAWN_TYPES);

const char *to_string(spawntype type)
{
    int i = static_cast<int>(type);
    if (i < 0 || i >= NTYPE)
        core::die("Invalid spawn type");
    return SPAWN_TYPES[i];
}

spawntype from_string(const std::string &type)
{
    for (int i = 0; i < NTYPE; i++) {
        if (type == SPAWN_TYPES[i])
            return static_cast<spawntype>(i);
    }
    std::printf("Unknown entity type: %s\n", type.c_str());
    core::die("Could not read level");
}

} // namespace spawninfo

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

void state::read_level()
{
    std::string path = level_path();
    FILE *fp = std::fopen(path.c_str(), "r");
    if (!fp) {
        if (errno == ENOENT) {
            std::printf("Level file is missing: %s\n", path.c_str());
            if (!edit_mode_)
                core::die("Could not load level");
            std::puts("Creating blank level");
            return;
        } else {
            core::die("Error when loading level");
        }
    }

    char linebuf[256], *linep;
    std::string whitespace = " \n\t\r";
    while ((linep = std::fgets(linebuf, sizeof(linebuf), fp)) != nullptr) {
        std::string line(linebuf);
        std::size_t pos = line.find_first_not_of(whitespace), end;
        if (pos == std::string::npos || line[pos] == '#')
            continue;
        std::string fields[4];
        // std::puts("parseline");
        for (int i = 0; i < 3; i++) {
            end = line.find_first_of(whitespace, pos);
            // std::printf("pos = %d..%d\n", (int)pos, (int)end);
            fields[i] = line.substr(pos, end - pos);
            if (end == std::string::npos)
                break;
            pos = line.find_first_not_of(whitespace, end);
            if (pos == std::string::npos)
                break;
        }
        if (pos != std::string::npos) {
            end = line.find_last_not_of(whitespace);
            // std::printf("pos = %d..%d\n", (int)pos, (int)end);
            if (end > pos) {
                if (end != std::string::npos)
                    fields[3] = line.substr(pos, end + 1 - pos);
                else
                    fields[3] = line.substr(pos);
            }
        }
        if (false) {
            std::puts("LINE:");
            for (int i = 0; i < 4; i++) {
                if (!fields[i].empty())
                    std::printf("  %d: %s\n", i + 1, fields[i].c_str());
            }
        }

        for (int i = 0; i < 3; i++) {
            if (fields[i].empty()) {
                std::puts("Not enough fields");
                core::die("Could not read level");
            }
        }

        spawnpoint s;
        s.type = spawninfo::from_string(fields[2]);
        s.pos = vec2(std::stoi(fields[0]), std::stoi(fields[1]));
        s.data = std::move(fields[3]);
        spawnpoints_.push_back(std::move(s));
    }

    if (std::ferror(fp))
        core::die("Error when loading level");

    std::fclose(fp);
}

void state::write_level() const
{
    auto path = level_path();
    FILE *fp = std::fopen(path.c_str(), "w");
    std::fprintf(fp, "# Level: %s\n", path.c_str());
    for (auto i = spawnpoints_.begin(), e = spawnpoints_.end(); i != e; i++) {
        std::fprintf(
            fp, "%+5d %+5d %s",
            (int)std::round(i->pos.x),
            (int)std::round(i->pos.y),
            spawninfo::to_string(i->type));
        if (!i->data.empty()) {
            std::fputc(' ', fp);
            std::fputs(i->data.c_str(), fp);
        }
        std::fputc('\n', fp);
    }
}

std::string state::level_path() const
{
    std::string path("level/");
    path += levelname_;
    path += ".txt";
    return path;
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
    levelname_ = name;

    spawnpoints_.clear();
    entities_.clear();
    new_entities_.clear();
    control_.clear();

    read_level();
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
