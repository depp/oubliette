/* Copyright 2013-2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_FILE_HPP
#define LD_FILE_HPP

#include <cstddef>
#include <cstdlib>
#include <string>

/// Read-only data buffer object.
class data {
private:
    void *ptr_;
    std::size_t size_;

public:
    data();
    data(const data &other) = delete;
    data(data &&other);
    ~data();
    data &operator=(const data &other) = delete;
    data &operator=(data &&other);

    /// Get the start of the buffer.
    const void *ptr() const { return ptr_; }
    /// Get the number of bytes in the buffer.
    std::size_t size() const { return size_; }
    /// Read the contents of a file.  Returns false on failure.
    static bool read(data *outdata, const std::string &path);
};

#endif
