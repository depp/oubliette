/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "script.hpp"
#include "audio.hpp"
#include "color.hpp"
#include "control.hpp"
#include "graphics.hpp"
#include "base/defs.hpp"
#include "defs.hpp"
#include <cstdio>
namespace script {

static const int LINETIME = 30;

using game::defs;
using ::graphics::color;

script::script()
{
    FILE *fp = fopen("script.txt", "r");
    if (!fp)
        core::die("Could not open script");

    section *sec = nullptr;

    std::string whitespace = " \n\t\r";
    char linebuf[64], *linep;
    while ((linep = std::fgets(linebuf, sizeof(linebuf), fp)) != nullptr) {
        std::string str(linebuf);
        auto end = str.find_last_not_of(whitespace);
        str.resize(end + 1);
        if (str.empty() || str[0] == '#')
            continue;
        if (str[0] == '@') {
            std::string name = str.substr(2);
            auto r = sections_.emplace(name, section());
            if (!r.second) {
                std::printf("Duplicate section: %s\n", name.c_str());
                core::die("Invalid script");
            }
            sec = &r.first->second;
            sec->name = name;
            continue;
        }
        if (sec == nullptr)
            core::die("Invalid script");
        if (str[0] == ' ') {
            if (sec->lines.empty())
                core::die("Invalid script");
            auto &l = sec->lines.back();
            l.lines++;
            l.text += '\n';
            l.text += str.substr(3);
            continue;
        }
        int color = -1;
        switch (str[0]) {
        case 'A': color = 8; break;
        case 'B': color = 9; break;
        case 'G': color = 15; break;
        case 'M':
            sec->track = str.substr(3);
            break;
        default:
            core::die("Invalid script");
        }
        if (color == -1)
            continue;
        sec->lines.emplace_back();
        auto &l = sec->lines.back();
        l.color = color;
        l.lines = 1;
        l.text = str.substr(3);
    }

    std::fclose(fp);
}

script::~script()
{ }

const section *script::get_section(const std::string &name) const
{
    auto i = sections_.find(name);
    return i == sections_.end() ? nullptr : &i->second;
}

system::system(const section &sec, const ::game::control_system &control,
               ::audio::system &audio)
    : m_section(sec), m_control(control), m_audio(audio), m_initted(false),
      m_lineno(0), m_linetime(0),
      m_revealed(false), m_done(false)
{
    if (!sec.track.empty())
        m_audio.play_music(sec.track, false);
}

system::~system()
{ }

void system::update()
{
    if (m_control.any_key_instant()) {
        next();
    } else if (!m_revealed) {
        m_linetime++;
        if (m_linetime == LINETIME)
            next();
    }
}

void system::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;

    if (!m_initted) {
        m_initted = true;
        gr.clear_text();
        int ypos = core::PHEIGHT - 8;
        int xpos = 8;
        auto &lines = m_section.lines;
        m_blocks.clear();
        m_blocks.reserve(lines.size());
        for (auto i = lines.begin(), e = lines.end(); i != e; i++) {
            m_blocks.push_back(gr.add_text(i->text, xpos, ypos));
            ypos -= 16 * i->lines + 8;
        }
    }

    for (int i = 0, e = m_blocks.size(); i != e; i++) {
        auto &l = m_section.lines[i];
        color text_color;
        if (i < m_lineno) {
            text_color = color::palette(l.color);
        } else if (i > m_lineno) {
            text_color = color::transparent();
        } else {
            text_color = color::palette(l.color).fade(
                (float)m_linetime * (1.0f / LINETIME)
                + (float)reltime * (1.0f / (LINETIME * defs::FRAMETIME)));
        }
        gr.set_text_color(m_blocks[i], text_color);
    }
}

void system::next()
{
    if (!m_revealed) {
        m_lineno++;
        m_linetime = 0;
        if ((std::size_t)m_lineno >= m_section.lines.size())
            m_revealed = true;
    } else {
        m_done = true;
    }
}

}
