/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_ALLOCATOR_HPP
#define LD_GAME_ALLOCATOR_HPP

#include <vector>
namespace game {

class allocator_system {
private:
    std::vector<int> freelist_;
    int next_;

public:
    allocator_system()
        : next_(0)
    { }

    int create();
    void destroy(int object);
    void clear();
};

}

#endif
