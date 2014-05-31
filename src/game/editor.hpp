/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_EDITOR_HPP
#define LD_GAME_EDITOR_HPP
#include <vector>
#include <string>
#include "base/vec.hpp"
#include "leveldata.hpp"
namespace graphics {
class system;
}
namespace game {
struct spawnpoint;
class control_system;

/// The level editor.
class editor_system {
private:
    const control_system &control_;
    const std::string levelname_;
    std::vector<spawnpoint> entities_;
    int selection_;
    fvec camera_pos_, camera_lastpos_;
    bool dragging_, panning_;
    ivec click_;
    spawntype type_;
    bool dirty_;
    int savetime_;

    /// Get the selection under the given point.
    int hit(ivec pos);
    /// Convert window to world coordinates.
    ivec window_to_world(ivec window_pos);
    /// Mark the editor as dirty.
    void mark_dirty();
    /// Sort entities.
    void sort();
    /// Delete the selection.
    void delete_object();
    /// Cycle the selection's type.
    void cycle_type(int dir);

public:
    editor_system(const control_system &control,
                  const std::string &levelname);
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
    void load_data();
    /// Save level data to disk.
    void save_data();
    /// Handle a mouse click event, or button == -1 for release.
    void mouse_click(ivec pos, int button);
    /// Handle a mouse movement event.
    void mouse_move(ivec pos);

    /// Are there unsaved changes?
    bool dirty() const { return dirty_; }
};

}
#endif
