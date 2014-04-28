/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_SCRIPT_HPP
#define LD_GAME_SCRIPT_HPP
#include <string>
#include <unordered_map>
#include <vector>
namespace game {
struct control_system;
}
namespace graphics {
struct system;
}
namespace script {

struct line {
    int color;
    int lines;
    std::string text;
};

struct section {
    std::string name;
    std::vector<line> lines;
};

class script {
private:
    std::unordered_map<std::string, section> sections_;

public:
    script();
    script(const script &) = delete;
    script(script &&) = delete;
    ~script();
    script &operator=(const script &) = delete;
    script &operator=(script &&) = delete;

    const section *get_section(const std::string &name) const;
};

class system {
private:
    const section &m_section;
    const ::game::control_system &m_control;
    bool m_initted;
    std::vector<int> m_blocks;
    int m_lineno;
    int m_linetime;
    bool m_done;

    /// Go to the next line.
    void next();

public:
    system(const section &sec, const ::game::control_system &control);
    system(const script &) = delete;
    system(system &&) = delete;
    ~system();
    system &operator=(const system &) = delete;
    system &operator=(system &&) = delete;

    /// Update the script.
    void update();
    /// Draw the script state to the screen.
    void draw(::graphics::system &gr, int reltime);
};

}
#endif
