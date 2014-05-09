/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "leveldata.hpp"
#include "graphics.hpp"
#include "../defs.hpp"
#include <cstdio>
#include <errno.h>
namespace game {
using ::graphics::sprite;
using ::graphics::ui;
using ::graphics::anysprite;

struct spawninfo {
    char name[8];
    int width;
    int height;
    int order;
    anysprite sp;
};

static const spawninfo SPAWN_TYPES[leveldata::NTYPE] = {
    { "player", 16, 24, 30, sprite::PLAYER },
    { "door",   24, 32, 10, sprite::DOOR2 },
    { "chest",  24, 24, 15, sprite::CHEST },
    { "slime",  16, 16, 20, sprite::SLIME1 },
    { "prof",   12, 24, 20, sprite::PROFESSOR },
    { "woman",  12, 24, 20, sprite::WOMAN },
    { "priest", 12, 24, 20, sprite::PRIEST },
    { "glyph",  16, 16, 25, ui::TREASURE01 }
};

static const spawninfo &get_spawninfo(spawntype type)
{
    int i = static_cast<int>(type);
    if (i < 0 || i >= leveldata::NTYPE)
        core::die("Invalid spawn type");
    return SPAWN_TYPES[i];
}

void spawnpoint::draw(::graphics::system &gr) const
{
    auto &s = get_spawninfo(type);
    gr.add_sprite(
        s.sp,
        vec2(x - s.width/2, y - s.height/2),
        ::sprite::orientation::NORMAL);
}

irect spawnpoint::bounds() const
{
    auto &s = get_spawninfo(type);
    return irect::centered(s.width, s.height).offset(x, y);
}

int spawnpoint::order() const
{
    return get_spawninfo(type).order;
}

bool spawnpoint::operator<(const struct spawnpoint &other) const
{
    return order() < other.order();
}

const char *leveldata::type_to_string(spawntype type)
{
    return get_spawninfo(type).name;
}

spawntype leveldata::type_from_string(const std::string &type)
{
    for (int i = 0; i < NTYPE; i++) {
        if (type == SPAWN_TYPES[i].name)
            return static_cast<spawntype>(i);
    }
    std::printf("Unknown entity type: %s\n", type.c_str());
    core::die("Could not read level");
}

std::vector<spawnpoint> leveldata::read_level(
    const std::string &levelname)
{
    std::vector<spawnpoint> data;
    std::string path = level_path(levelname);
    FILE *fp = std::fopen(path.c_str(), "r");
    if (!fp) {
        if (errno == ENOENT) {
            std::printf("Level file is missing: %s\n", path.c_str());
            return data;
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
        s.x = std::stoi(fields[0]);
        s.y = std::stoi(fields[1]);
        s.type = type_from_string(fields[2]);
        s.data = std::move(fields[3]);
        data.push_back(std::move(s));
    }

    if (std::ferror(fp))
        core::die("Error when loading level");

    std::fclose(fp);
    return std::move(data);
}

void leveldata::write_level(
    const std::string &levelname,
    const std::vector<spawnpoint>& data)
{
    auto path = level_path(levelname);
    FILE *fp = std::fopen(path.c_str(), "w");
    std::fprintf(fp, "# Level: %s\n", path.c_str());
    for (auto i = data.begin(), e = data.end(); i != e; i++) {
        std::fprintf(
            fp, "%+5d %+5d %s",
            i->x, i->y, type_to_string(i->type));
        if (!i->data.empty()) {
            std::fputc(' ', fp);
            std::fputs(i->data.c_str(), fp);
        }
        std::fputc('\n', fp);
    }
    std::fclose(fp);
}

std::string leveldata::level_path(const std::string &levelname)
{
    if (levelname.empty())
        core::die("Empty level name");
    std::string path("level/");
    path += levelname;
    path += ".txt";
    return path;
}

}
