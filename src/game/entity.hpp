/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_ENTITY_HPP
#define LD_GAME_ENTITY_HPP
#include "vec.hpp"
namespace graphics {
class system;
}
namespace game {
class state;

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
    entity(state &st, team t);
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
    state &m_state;
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

    physics_component(irect bbox, vec2 pos, vec2 vel);

    /// Update the physics component of this entity.
    void update(state &st, entity &e);
    /// Get the position at the given time since the last update.
    vec2 get_pos(int reltime);
};

/// Walking component for an entity.
class walking_component {
public:
    vec2 gravity;
    float xmove;
    float ymove;

    walking_component();

    /// Update the walking component of this entity.
    void update(state &st, physics_component &physics);
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
    player(state &st, vec2 pos);
    virtual ~player();

    virtual void update();
    virtual void damage(int amount);
    virtual void draw(::graphics::system &gr, int reltime);
};

}

#endif
