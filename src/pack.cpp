/* Copyright 2013-2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <assert.h>
#include "defs.hpp"
#include "pack.hpp"
namespace pack {

// Rect, sorts from biggest to smallest.
struct rect {
    size rectsize;
    std::size_t index;

    bool operator<(const rect &other) const;
};

bool rect::operator<(const rect &other) const
{
    if (rectsize.width > other.rectsize.width)
        return true;
    if (rectsize.width < other.rectsize.width)
        return false;
    return rectsize.height > other.rectsize.width;
}

// Metrics for how good a packing is.
struct metrics {
    int area;
    int nonsquare;

    metrics();
    metrics(const size &size);
    bool operator<(const metrics &other) const;
};

metrics::metrics()
{
}

metrics::metrics(const size &size)
    : area(size.width * size.height),
      nonsquare(std::abs(size.width - size.height))
{
}

bool metrics::operator<(const metrics &other) const
{
    if (area < other.area)
        return true;
    if (area > other.area)
        return false;
    return nonsquare < other.nonsquare;
}

// Packing state.
struct packer {
    std::vector<int> frontier;
    packing result;
    std::vector<rect> rects;

    bool try_pack(int width, int height);
};

bool packer::try_pack(int width, int height)
{
    frontier.clear();
    frontier.resize(height, 0);
    result.locations.resize(rects.size());
    result.packsize.width = width;
    result.packsize.height = height;
    int ypos = 0;
    for (auto rect : rects) {
        int sw = rect.rectsize.width, sh = rect.rectsize.height;
        std::size_t index = rect.index;
        if (ypos + sw > height)
            ypos = 0;
        while (ypos + sh <= height && frontier[ypos] + sw > width)
            ypos++;
        if (ypos + sh > height)
            return false;
        int xpos = frontier[ypos];
        for (int i = 0; i < ypos + sh; i++) {
            if (frontier[i] < xpos + sw)
                frontier[i] = xpos + sw;
        }
        assert(0 <= ypos && ypos <= height - sh);
        assert(0 <= xpos && xpos <= width - sw);
        result.locations[index].x = xpos;
        result.locations[index].y = ypos;
        ypos += sh;
    }
    return true;
}

packing pack(const std::vector<size> &rects)
{
    packer packer;
    std::size_t rectarea = 0;
    packer.rects.reserve(rects.size());
    for (std::size_t i = 0; i < rects.size(); i++) {
        rect r;
        r.rectsize = rects[i];
        r.index = i;
        packer.rects.push_back(r);
        rectarea += static_cast<std::size_t>(rects[i].width) *
            static_cast<std::size_t>(rects[i].height);
    }
    std::sort(packer.rects.begin(), packer.rects.end());

    bool havepacking = false;
    packing bestpacking;
    for (int i = 4; i < 10; i++) {
        int height = 1 << i;
        bool success = false;
        for (int j = 4; j < 10; j++) {
            int width = 1 << j;
            if (rectarea > static_cast<std::size_t>(1) << (i + j))
                continue;
            if (packer.try_pack(width, height)) {
                success = true;
                break;
            }
        }
        if (!success)
            continue;
        if (havepacking) {
            metrics curmetrics(packer.result.packsize);
            metrics bestmetrics(bestpacking.packsize);
            if (curmetrics < bestmetrics) {
                packing temp(std::move(bestpacking));
                bestpacking = std::move(packer.result);
                packer.result = std::move(temp);
            }
        } else {
            bestpacking = std::move(packer.result);
            havepacking = true;
        }
    }
    if (!havepacking)
        core::die("Could not pack sprites.");
    return bestpacking;
}

}
