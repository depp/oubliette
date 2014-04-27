/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_LEVELDATA_HPP
#define LD_GAME_LEVELDATA_HPP
#include <vector>
#include <string>
#include "vec.hpp"
namespace graphics {
class system;
}
namespace game {
struct spawnpoint;

/// Types of objects that can be spawned at level start.
enum class spawntype {
    PLAYER,
    DOOR,
    CHEST,
    SLIME,
    PROF,
    WOMAN,
    PRIEST
};

/// Spawn point where an entity starts.
struct spawnpoint {
    int x, y;
    spawntype type;
    std::string data;

    // Flag for the editor.
    bool flag;

    /// Draw the spawn point.  This is only used by the editor.
    void draw(::graphics::system &gr) const;

    /// Get the bounds for this object.
    irect bounds() const;

    /// Get the sort order for this object.
    int order() const;

    bool operator<(const struct spawnpoint &other) const;
};

/// Level data utilities
struct leveldata {
    static const int NTYPE = static_cast<int>(spawntype::PRIEST) + 1;

    /// Convert a spawn type to a string.
    static const char *type_to_string(spawntype type);

    /// Convert a string to a spawn type.
    static spawntype type_from_string(const std::string &type);

    /// Read level data.
    static std::vector<spawnpoint> read_level(
        const std::string &levelname);

    /// Write level data.
    static void write_level(
        const std::string &levelname,
        const std::vector<spawnpoint>& data);

    /// Get the path to the level file.
    static std::string level_path(const std::string &levelname);
};

}
#endif
