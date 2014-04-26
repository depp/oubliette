/* Copyright 2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "allocator.hpp"
namespace game {

int allocator_system::create()
{
    int object;
    if (freelist_.empty()) {
        object = next_;
        next_++;
    } else {
        object = freelist_.back();
        freelist_.pop_back();
    }
    return object;
}

void allocator_system::destroy(int object)
{
    freelist_.push_back(object);
}

void allocator_system::clear()
{
    freelist_.clear();
    next_ = 0;
}

}
