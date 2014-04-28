/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_ENTITY_HPP
#define LD_GAME_ENTITY_HPP
#include "vec.hpp"
#include "camera.hpp"
#include "levelmap.hpp"
#include <string>
#include <vector>
#include <memory>
namespace graphics {
class system;
}
namespace game {
class entity;
struct control_system;

/// The entity system.
class entity_system {
private:
    /// The control system.
    const control_system &control_;
    /// The level name.
    const std::string levelname_;
    /// List of all entities in the game.
    std::vector<std::unique_ptr<entity>> entities_;
    /// List of pending new entities.
    std::vector<std::unique_ptr<entity>> new_entities_;
    /// The camera system.
    camera_system camera_;
    /// The level collision map.
    levelmap level_;
    /// List of points which trigger contextual hovering.
    std::vector<ivec> hover_triggers_;

    /// Spawn an entity for the initial level state.
    void spawn(const struct spawnpoint &data);

public:
    entity_system(const control_system &control,
                  const std::string &levelname);

    /// Update the entities.
    void update();
    /// Draw the game state to the screen.
    void draw(::graphics::system &gr, int reltime);
    /// Add an entity to the game.
    void add_entity(entity *ent);
    /// Set the camera target.
    void set_camera_target(const rect &target);
    /// Add a hover trigger.
    void set_hover(ivec pos);
    /// Test if there are hover triggers in the rect.
    bool test_hover(irect rect);

    const control_system &control() const { return control_; }
    const levelmap &level() const { return level_; }
    const std::vector<std::unique_ptr<entity>> &entities() const
    { return entities_; }

    /// A hack... set this, and that level will be loaded.
    std::string nextlevel;
};

// ======================================================================
// Components and abstract entity parts
// ======================================================================

/// Teams for entities.
enum class team {
    /// The entity is dead, and should be removed.
    DEAD,

    /// No special properties, such as decorative objects.
    AMBIENT,

    /// Player can interact with this object.  Causes contextual arrow.
    INTERACTIVE,

    /// The player, enemies will attack this.
    FRIEND,

    /// The player's shots, enemies will avoid this.
    FRIEND_SHOT,

    /// Enemies.
    FOE,

    /// The enemies' shots.
    FOE_SHOT
};

/// Generic game entity superclass.
class entity {
public:
    entity(entity_system &sys, team t);
    entity(const entity &) = delete;
    entity(entity &&) = delete;
    virtual ~entity();
    entity &operator=(const entity &) = delete;
    entity &operator=(entity &&) = delete;

    /// Update the entity's state for the next frame.
    virtual void update();
    /// Handle the player interacting with the object.
    virtual void interact();
    /// Damage the object.
    virtual void damage(int amount);
    /// Draw the sprite to the graphics system.
    virtual void draw(::graphics::system &gr, int reltime) = 0;

    /// Link to the enclosing world state.
    entity_system &m_system;
    /// The bounding box, in world coordinates.
    irect m_bbox;
    /// The entity's team.
    team m_team;
};

/// Physics component for an entity.
class physics_component {
public:
    // Only the accel should be changed by others.
    irect bbox;
    vec2 lastpos;
    vec2 pos;
    vec2 vel;
    vec2 accel;
    bool on_floor;

    physics_component(irect bbox, vec2 pos, vec2 vel);

    /// Update the physics component of this entity.
    void update(entity_system &sys, entity &e);
    /// Get the position at the given time since the last update.
    vec2 get_pos(int reltime);
};

enum jumpstate {
    READY, JUMP1, JUMP2
};

/// Walking statistics.
struct walking_stats {
    float gravity;
    float xaccel_ground;
    float xspeed_ground;
    float xaccel_air;
    float xspeed_air;
    int floordepth;
    int jumptime;
    float jumpaccel;
    float jumpspeed;
    bool can_doublejump;
};

/// Walking component for an entity.
class walking_component {
public:
    vec2 gravity;
    float xmove;
    float ymove;
    int jumptime;
    jumpstate jstate;

    walking_component();

    /// Update the walking component of this entity.
    void update(entity_system &sys, physics_component &physics,
                const walking_stats &stats);
};

// ======================================================================
// Concrete entities
// ======================================================================

/// The player.
class player : public entity {
private:
    physics_component physics;
    walking_component walking;

public:
    player(entity_system &sys, vec2 pos);
    virtual ~player();

    virtual void update();
    virtual void damage(int amount);
    virtual void draw(::graphics::system &gr, int reltime);
};

/// Doors between areas.
class door : public entity {
public:
    const vec2 m_pos;
    const std::string m_target;

    door(entity_system &sys, vec2 pos, const std::string target);
    virtual ~door();

    /// Handle the player interacting with the object.
    virtual void interact();
    /// Draw the sprite to the graphics system.
    virtual void draw(::graphics::system &gr, int reltime);
};

}
#endif
