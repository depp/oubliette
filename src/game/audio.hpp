/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_AUDIO_HPP
#define LD_GAME_AUDIO_HPP
#include <string>
#include <vector>
namespace audio {

enum class sfx {
    DOOR_OPEN,
    ENEMY_DIE,
    ENEMY_HIT,
    ENEMY_SHOOT,
    PLAYER_HIT,
    PLAYER_JUMP,
    PLAYER_SHOOT,
    PLAYER_STEP,
    SHOT_IMPACT
};

static const int SFX_COUNT = static_cast<int>(sfx::SHOT_IMPACT) + 1;

/// The audio system.
class system {
private:
    struct wave;
    struct track_info;

    std::vector<wave> sfx_wave_;
    std::vector<track_info> tracks_;
    int sfx_channel_;

    void load_trackinfo();
    void load_sfx();

public:
    system();
    system(const system &) = delete;
    system(system &&) = delete;
    ~system();
    system &operator=(const system &) = delete;
    system &operator=(system &&) = delete;

    /// Set the current music track.  Set to empty to stop music.
    void play_music(const std::string &name, bool do_loop);

    /// Play a sound effect.
    void play_sfx(sfx s);
};

}
#endif
