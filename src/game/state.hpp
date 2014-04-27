/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_STATE_HPP
#define LD_GAME_STATE_HPP
#include <memory>
#include <string>
#include <vector>
#include "camera.hpp"
#include "control.hpp"
#include "key.hpp"
#include "levelmap.hpp"
#include "graphics.hpp"
namespace graphics {
class system;
}
namespace game {
class entity;
class editor_system;

/// State of the game world.
class state {
private:
    /// Whether the world is in edit mode.
    bool edit_mode_;
    /// Whether the state has been fully initialized.
    bool initted_;
    /// The timestamp of the last update.
    unsigned frametime_;
    /// The current level name.
    std::string levelname_;
    /// List of all entities in the game.
    std::vector<std::unique_ptr<entity>> entities_;
    /// List of pending new entities.
    std::vector<std::unique_ptr<entity>> new_entities_;
    /// The control (i.e. player input) system.
    control_system control_;
    /// The level collision map.
    levelmap level_;
    /// The camera system.
    camera_system camera_;
    /// The graphics system.
    graphics::system graphics_;
    /// The editor system.
    std::unique_ptr<editor_system> editor_;

    /// Advance to the given frame.
    void advance(unsigned time);

public:
    explicit state(bool edit_mode);
    state(const state &) = delete;
    state(state &&) = delete;
    ~state();
    state &operator=(const state &) = delete;
    state &operator=(state &&) = delete;

    /// Draw the game state to the screen.
    void draw(unsigned time);
    /// Handle a mouse click event, or button == -1 for release.
    void mouse_click(int x, int y, int button);
    /// Handle a mouse movement event.
    void mouse_move(int x, int y);
    /// Handle a keyboard event.
    void event_key(key k, bool state);
    /// Set the current level.
    void set_level(const std::string &name);
    /// Add an entity to the game.
    void add_entity(std::unique_ptr<entity> &&ent);
    void add_entity(entity *ent);
    /// Set the camera target.
    void set_camera_target(const rect &target);

    const control_system control() const { return control_; }
    const levelmap &level() const { return level_; }
};

}
#endif
