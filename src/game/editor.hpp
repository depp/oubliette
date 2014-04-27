/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_EDITOR_HPP
#define LD_GAME_EDITOR_HPP
#include <vector>
#include <string>
#include "vec.hpp"
#include "leveldata.hpp"
namespace graphics {
class system;
}
namespace game {
class state;
struct spawnpoint;

/// The level editor.
class editor_system {
private:
    const state &state_;
    std::vector<spawnpoint> entities_;
    int selection_;
    vec2 camera_pos_, camera_lastpos_;
    bool dragging_;
    int clickx_, clicky_;
    spawntype type_;

    /// Get the selection under the given point.
    int hit(int x, int y);
    /// Convert window to world coordinates.
    void window_to_world(int &x, int &y);

public:
    explicit editor_system(const state &st);
    editor_system(const editor_system &) = delete;
    editor_system(editor_system &&) = delete;
    ~editor_system();
    editor_system &operator=(const editor_system &) = delete;
    editor_system &operator=(editor_system &&) = delete;

    /// Update the editor state.
    void update();
    /// Draw the editor data.
    void draw(::graphics::system &gr, int reltime);
    /// Load the level data into the editor.
    void load_data(std::vector<spawnpoint> &&data);
    /// Save level data to disk.
    void save_data(const std::string &levelname);
    /// Handle a mouse click event, or button == -1 for release.
    void mouse_click(int x, int y, int button);
    /// Handle a mouse movement event.
    void mouse_move(int x, int y);
};

}
#endif
