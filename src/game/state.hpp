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
#include "persistent.hpp"
namespace graphics {
class system;
}
namespace script {
class script;
class system;
}
namespace game {
class entity_system;
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
    /// The control (i.e. player input) system.
    control_system control_;
    /// The graphics system.
    graphics::system graphics_;
    /// The entity system.
    std::unique_ptr<entity_system> entity_;
    /// The editor system.
    std::unique_ptr<editor_system> editor_;
    /// Persistent state.
    persistent_state persistent_;
    /// The game script.
    std::unique_ptr<script::script> script_;
    /// The script system.
    std::unique_ptr<script::system> scriptsys_;
    /// Queue for level changes.
    std::vector<std::string> levelqueue_;

    /// Advance to the given frame.
    void advance(unsigned time);
    /// Go to the next level.
    void next_level();

public:
    explicit state(bool edit_mode);
    state(const state &) = delete;
    state(state &&) = delete;
    ~state();
    state &operator=(const state &) = delete;
    state &operator=(state &&) = delete;

    /// Set the current level.
    void set_level(const std::string &levelname);
    /// Draw the game state to the screen.
    void draw(unsigned time);
    /// Handle a mouse click event, or button == -1 for release.
    void mouse_click(int x, int y, int button);
    /// Handle a mouse movement event.
    void mouse_move(int x, int y);
    /// Handle a keyboard event.
    void event_key(key k, bool state);

};

}
#endif
