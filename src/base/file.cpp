/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "file.hpp"
#include <cstdlib>
#include <cstdio>
#include <new>

data::data()
    : ptr_(nullptr), size_(0)
{
}

data::data(data &&other)
    : ptr_(nullptr), size_(0)
{
    ptr_ = other.ptr_;
    size_ = other.size_;
    other.ptr_ = nullptr;
    other.size_ = 0;
}

data::~data()
{
    std::free(ptr_);
}

data &data::operator=(data &&other)
{
    if (this == &other)
        return *this;
    std::free(ptr_);
    ptr_ = other.ptr_;
    size_ = other.size_;
    other.ptr_ = nullptr;
    other.size_ = 0;
    return *this;
}

bool data::read(data *outdata, const std::string &path)
{
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) {
        std::fprintf(stderr, "Could not open file: %s\n", path.c_str());
        return false;
    }
    try {
        static const std::size_t INITSZ = 4096;
        data data;
        std::size_t alloc = INITSZ;
        data.ptr_ = std::malloc(alloc);
        if (!data.ptr_)
            core::die_alloc();
        std::size_t pos = 0;
        while (1) {
            std::size_t amt = std::fread(
                static_cast<char *>(data.ptr_) + pos,
                1, alloc - pos, fp);
            if (amt == 0) {
                if (std::feof(fp)) {
                    data.size_ = pos;
                    std::fclose(fp);
                    *outdata = std::move(data);
                    return true;
                }
                std::fprintf(stderr, "Could not read file: %s\n",
                             path.c_str());
                return false;
            }
            pos += amt;
            if (pos >= alloc) {
                alloc *= 2;
                if (!alloc) {
                    std::fprintf(stderr, "File too large: %s\n",
                                 path.c_str());
                    return false;
                }
                void *nptr = realloc(data.ptr_, alloc);
                if (!nptr)
                    core::die_alloc();
            }
        }
    } catch (...) {
        std::fclose(fp);
        throw;
    }
}
