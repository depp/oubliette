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
enum class sprite;
}
namespace game {
class entity;
struct control_system;
struct walking_stats;
struct jumping_stats;
struct enemy_stats;
struct persistent_state;

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

/// The entity system.
class entity_system {
private:
    /// Game persistent state.
    persistent_state &state_;
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
    /// Point which triggers hovering.
    ivec hover_trigger_;
    /// The last camera position.
    ivec lastcamera_;
    /// The last mouse click pos.
    ivec clickpos_;
    /// Whether we clicked the mouse.
    bool is_click_;

public:
    entity_system(persistent_state &state,
                  const control_system &control,
                  const std::string &levelname,
                  const std::string &lastlevel);

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
    /// Scan for a target in the given range, on the given team.
    entity *scan_target(irect range, team t);
    /// Record player death.
    void die();
    /// Handle a mouse click.
    void mouse_click(int x, int y, int button);
    /// Spawn a projectile.
    void spawn_shot(team t, vec2 origin, vec2 target, float speed,
                    ::graphics::sprite sp1, ::graphics::sprite sp2);

    const control_system &control() const { return control_; }
    const levelmap &level() const { return level_; }
    const std::vector<std::unique_ptr<entity>> &entities() const
    { return entities_; }
    persistent_state &state() { return state_; }
    ivec clickpos() const { return clickpos_; }
    bool is_click() const { return is_click_; }

    /// A hack... set this, and that level will be loaded.
    std::string nextlevel;
};

// ======================================================================
// Components and abstract entity parts
// ======================================================================

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
    /// Get the entity's center.
    vec2 center() const;

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

/// Physics for projectile entities.
class projectile_component {
public:
    irect bbox;
    vec2 lastpos;
    vec2 pos;
    vec2 vel;
    bool is_hit;
    int damage;

    projectile_component(irect bbox, vec2 pos, vec2 vel, int damage);

    /// Update the physics component of this entity.
    void update(entity_system &sys, entity &e);
    /// Handle a collision.
    void hit(entity_system &sys, entity &e);
    /// Get the position at the given time since the last update.
    vec2 get_pos(int reltime);
};

enum jumpstate {
    READY, JUMP1, JUMP2
};

/// Walking component for an entity.
class walking_component {
public:
    float xmove;

    walking_component();

    /// Update the walking component of this entity.
    void update(physics_component &physics,
                const walking_stats &stats);
};

/// Jumping component with full control (for the player).
class jumping_component {
public:
    float ymove;
    int jumptime;
    jumpstate jstate;

    jumping_component();

    void update(physics_component &physics,
                const jumping_stats &stats);
};

/// Enemy state component.
class enemy_component {
public:
    enum class state { IDLE, ALERT, ATTACK };
    state m_state;
    int m_time;
    vec2 m_targetpos;

    enemy_component();

    void update(entity_system &sys, entity &e,
                const enemy_stats &stats);

    entity *scan(entity_system &sys, entity &e,
                 const enemy_stats &stats);

    bool start_attack() const
    { return m_state == state::ATTACK && m_time == 0; }
};

// ======================================================================
// Concrete entities
// ======================================================================

/// The player.
class player : public entity {
private:
    physics_component physics;
    walking_component walking;
    jumping_component jumping;

public:
    player(entity_system &sys, vec2 pos);
    virtual ~player();

    virtual void update();
    virtual void damage(int amount);
    virtual void draw(::graphics::system &gr, int reltime);
};

/// Doors between areas.
class door : public entity {
private:
    const vec2 m_pos;
    const std::string m_target;
    bool m_is_locked;

public:
    door(entity_system &sys, vec2 pos, const std::string target);
    virtual ~door();

    virtual void interact();
    virtual void draw(::graphics::system &gr, int reltime);
};

/// Treasure chest.
class chest : public entity {
private:
    const vec2 m_pos;
    int m_which, m_state;

public:
    chest(entity_system &sys, vec2 pos, const std::string &contents);
    virtual ~chest();

    virtual void interact();
    virtual void draw(::graphics::system &gr, int reltime);
};

/// Enemy.
class enemy : public entity {
private:
    enemy_component m_enemy;
    physics_component physics;
    walking_component walking;
    ::graphics::sprite m_actor;
    ::graphics::sprite m_shot1;
    ::graphics::sprite m_shot2;

public:
    enemy(entity_system &sys, vec2 pos,
          ::graphics::sprite actor,
          ::graphics::sprite shot1, ::graphics::sprite shot2);
    virtual ~enemy();

    virtual void update();
    virtual void draw(::graphics::system &gr, int reltime);
};

/// Projectiles.
class shot : public entity {
private:
    projectile_component projectile;
    int time;
    ::graphics::sprite m_sp1;
    ::graphics::sprite m_sp2;

public:
    shot(entity_system &sys, team t, vec2 pos, vec2 vel, int time,
         ::graphics::sprite sp1, ::graphics::sprite sp2);
    virtual ~shot();

    virtual void update();
    virtual void draw(::graphics::system &gr, int reltime);
};

/// Projectile poof.
class poof : public entity {
public:
    vec2 m_pos;
    int m_time;
    int m_frame;

    poof(entity_system &sys, vec2 pos);
    virtual ~poof();

    virtual void update();
    virtual void draw(::graphics::system &gr, int reltime);
};

}
#endif
