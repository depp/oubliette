/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "audio.hpp"
#include "../defs.hpp"
#include <SDL_mixer.h>
#include <cstdio>
#include <cmath>
namespace audio {

static const int SFX_CHANNELS = 6;
static const int FADE_OUT_TIME = 250;

/// Convert decibels to magnitude (voltage ratio, not power ratio).
static double db_to_linear(double db)
{
    double factor = std::log(10.0) / 20.0;
    return std::exp(db * factor);
}

static int convert_volume(double gain)
{
    int volume = std::floor(MIX_MAX_VOLUME * gain + 0.5);
    if (volume <= 0)
        volume = 1;
    else if (volume > MIX_MAX_VOLUME)
        volume = MIX_MAX_VOLUME;
    return volume;
}

static const char WAVE_NAMES[SFX_COUNT][13] = {
    "door_open",
    "enemy_die",
    "enemy_hit",
    "enemy_shoot",
    "player_hit",
    "player_jump",
    "player_shoot",
    "player_step",
    "shot_impact"
};

struct system::wave {
    Mix_Chunk *data;
    double volume;

    wave() : data(nullptr), volume(1.0) { }
};

struct system::track_info {
    std::string name;
    double loop_length;
    Mix_Chunk *data;

    track_info() : loop_length(-1.0), data(nullptr) { }
};

static const std::string WHITESPACE(" \n\t\r");

// Parse a line containing a string and a number.
static int parse_line(const std::string &input,
                      std::string &name, double &data)
{
    std::string fields[2];
    std::size_t pos = 0;
    for (int i = 0; i < 2 && pos != std::string::npos; i++) {
        std::size_t start = input.find_first_not_of(WHITESPACE, pos);
        if (start == std::string::npos)
            break;
        std::size_t end = input.find_first_of(WHITESPACE, start);
        fields[i] = input.substr(
            start,
            end != std::string::npos ? end - start : std::string::npos);
        pos = end;
    }

    if (fields[0].empty())
        return 0;
    if (fields[1].empty())
        return -1;
    double value = std::stod(fields[1]);
    name = std::move(fields[0]);
    data = value;
    return 0;
}

system::system()
    : sfx_channel_(0), music_channel_(0)
{
    for (int i = 0; i < MUSIC_CHANNELS; i++)
        music_tracks_[i] = false;
    load_sfx();
    load_trackinfo();
}

system::~system()
{
}

void system::load_sfx()
{
    sfx_wave_.reserve(SFX_COUNT);
    for (int i = 0; i < SFX_COUNT; i++) {
        std::string path("sfx/");
        path += WAVE_NAMES[i];
        path += ".ogg";
        wave w;
        w.data = Mix_LoadWAV(path.c_str());
        if (!w.data)
            std::printf("Could not load %s\n", path.c_str());
        w.volume = 1.0;
        sfx_wave_.push_back(w);
    }

    FILE *fp = std::fopen("sfx/volume.txt", "r");
    if (!fp) {
        std::puts("Could not open sfx/volume.txt");
        return;
    }

    int lineno = 0;
    char linebuf[64], *linep;
    while ((linep = std::fgets(linebuf, sizeof(linebuf), fp)) != nullptr) {
        lineno++;
        std::string line(linebuf), name;
        double volume;
        int result = parse_line(line, name, volume);
        if (result > 0) {
            int i = 0;
            for (; i < SFX_COUNT; i++) {
                if (name == WAVE_NAMES[i]) {
                    sfx_wave_.at(i).volume = db_to_linear(volume);
                    break;
                }
            }
            if (i >= SFX_COUNT)
                std::printf("sfx/volume.txt:%d: no such sound effect\n",
                            lineno);
        } else if (result == 0) {
            continue;
        } else {
            std::printf("sfx/volume.txt:%d: could not parse\n", lineno);
            break;
        }
    }
}

void system::load_trackinfo()
{
    FILE *fp = std::fopen("music/looplength.txt", "r");
    if (!fp) {
        std::puts("Could not open music/looplength.txt");
        return;
    }

    int lineno = 0;
    char linebuf[64], *linep;
    while ((linep = std::fgets(linebuf, sizeof(linebuf), fp)) != nullptr) {
        lineno++;
        std::string line(linebuf);
        track_info info;
        int result = parse_line(line, info.name, info.loop_length);
        if (result > 0) {
            tracks_.push_back(std::move(info));
        } else if (result == 0) {
            continue;
        } else {
            std::printf("music/looplength.txt:%d: could not parse\n", lineno);
            break;
        }
    }
}

void system::play_music(const std::string &name, bool one_shot)
{
    int new_track = -1;
    if (!name.empty()) {
        for (std::size_t i = 0, n = tracks_.size(); i < n; i++) {
            if (tracks_[i].name == name) {
                new_track = (int)i;
                break;
            }
        }
        if (new_track == -1) {
            new_track = (int)tracks_.size();
            track_info new_info;
            new_info.name = name;
            tracks_.push_back(std::move(new_info));
        }
    }

    int &playing = music_tracks_[music_channel_];
    if (playing >= 0) {
        if (!Mix_Playing(SFX_CHANNELS + music_channel_))
            playing = -1;
    }
    if (new_track == playing)
        return;
    if (playing >= 0) {
        if (!music_one_shot_)
            Mix_FadeOutChannel(SFX_CHANNELS + music_channel_, FADE_OUT_TIME);
        music_channel_ = !music_channel_;
        music_tracks_[music_channel_] = -1;
    }
    if (new_track >= 0) {
        track_info &info = tracks_[new_track];
        music_tracks_[music_channel_] = new_track;
        if (!info.data) {
            std::string path("music/");
            path += info.name;
            path += ".ogg";
            info.data = Mix_LoadWAV(path.c_str());
            if (!info.data)
                std::printf("Failed to load %s: %s\n",
                            path.c_str(), Mix_GetError());
        }
        if (info.data) {
            Mix_PlayChannel(SFX_CHANNELS + music_channel_, info.data,
                            one_shot ? 0 : -1);
            music_one_shot_ = one_shot;
        }
    }
}

void system::play_sfx(sfx s)
{
    int channel = sfx_channel_ + 1;
    if (channel >= SFX_CHANNELS)
        channel = 0;
    if (Mix_Playing(channel)) {
        std::puts("Too many sounds, dropped");
        return;
    }
    const wave &w = sfx_wave_.at((int)s);
    if (!w.data)
        return;
    Mix_Volume(channel, convert_volume(w.volume));
    Mix_PlayChannel(channel, w.data, 0);
    sfx_channel_ = channel;
}

}
