/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_ARRAY_HPP
#define LD_ARRAY_HPP

#include "opengl.hpp"
#include <limits>
#include <new>
#include <cstdlib>

namespace array {

// OpenGL attribute array data types.
template<typename T>
struct array_type { };

template<>
struct array_type<short> {
    static const GLenum TYPE = GL_SHORT;
    static const int SIZE = 1;
};

template<>
struct array_type<short[2]> {
    static const GLenum TYPE = GL_SHORT;
    static const int SIZE = 2;
};

template<>
struct array_type<short[3]> {
    static const GLenum TYPE = GL_SHORT;
    static const int SIZE = 3;
};

template<>
struct array_type<short[4]> {
    static const GLenum TYPE = GL_SHORT;
    static const int SIZE = 4;
};

template<>
struct array_type<float> {
    static const GLenum TYPE = GL_FLOAT;
    static const int SIZE = 1;
};

template<>
struct array_type<float[2]> {
    static const GLenum TYPE = GL_FLOAT;
    static const int SIZE = 2;
};

template<>
struct array_type<float[3]> {
    static const GLenum TYPE = GL_FLOAT;
    static const int SIZE = 3;
};

template<>
struct array_type<float[4]> {
    static const GLenum TYPE = GL_FLOAT;
    static const int SIZE = 4;
};

// OpenGL attribute array class.
template<class T>
class array {
private:
    T *data_;
    int count_;
    int alloc_;
    GLuint buffer_;

public:
    explicit array();
    array(const array &other) = delete;
    array(array &&other);
    ~array();
    array &operator=(const array &other) = delete;
    array &operator=(array &&other);

    /// Get the number of elements in the array.
    int size() const;
    /// Determine whether the array is empty.
    bool empty() const;
    /// Set the number of elements in the array to zero.
    void clear();
    /// Reserve space for the given total number of elements.
    void reserve(std::size_t total);
    /// Insert the given number of elements and return a pointer to the first.
    T *insert(std::size_t count);
    /// Upload the array to an OpenGL buffer.
    void upload(GLenum usage);
    /// Set the array as a vertex attribute.
    void set_attrib(GLint attrib);
};

template<class T>
inline array<T>::array()
    : data_(nullptr), count_(0), alloc_(0), buffer_(0)
{
}

template<class T>
inline array<T>::array(array<T> &&other)
    : data_(nullptr), count_(0), alloc_(0), buffer_(0)
{
    data_ = other.data_;
    count_ = other.count_;
    alloc_ = other.alloc_;
    buffer_ = other.buffer_;
    other.data_ = nullptr;
    other.count_ = 0;
    other.alloc_ = 0;
    other.buffer_ = 0;
}

template<class T>
inline array<T>::~array()
{
    std::free(data_);
    glDeleteBuffers(1, &buffer_);
}

template<class T>
array<T> &array<T>::operator=(array<T> &&other)
{
    if (this == &other)
        return *this;
    std::free(data_);
    data_ = other.data_;
    count_ = other.count_;
    alloc_ = other.alloc_;
    buffer_ = other.buffer_;
    other.data_ = nullptr;
    other.count_ = 0;
    other.alloc_ = 0;
    other.buffer_ = 0;
    return *this;
}

template<class T>
int array<T>::size() const
{
    return count_;
}

template<class T>
bool array<T>::empty() const
{
    return count_ == 0;
}

template<class T>
void array<T>::clear()
{
    count_ = 0;
}

template<class T>
void array<T>::reserve(std::size_t total)
{
    std::size_t rounded;
    if (total > std::numeric_limits<int>::max())
        throw std::bad_alloc();
    if (alloc_ >= total)
        return;
    rounded = total - 1;
    rounded |= rounded >> 1;
    rounded |= rounded >> 2;
    rounded |= rounded >> 4;
    rounded |= rounded >> 8;
    rounded |= rounded >> 16;
    rounded += 1;
    T *newdata = static_cast<T *>(std::realloc(data_, sizeof(T) * rounded));
    if (!newdata)
        throw std::bad_alloc();
    data_ = newdata;
    alloc_ = rounded;
}

template<class T>
T *array<T>::insert(std::size_t count)
{
    if (count > alloc_ - count_) {
        if (count > std::numeric_limits<int>::max() - count_)
            throw std::bad_alloc();
        reserve(count + count_);
    }
    int pos = count_;
    count_ += count;
    return data_ + pos;
}

template<class T>
void array<T>::upload(GLenum usage)
{
    if (buffer_ == 0)
        glGenBuffers(1, &buffer_);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_);
    glBufferData(GL_ARRAY_BUFFER, count_ * sizeof(T), data_, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<class T>
void array<T>::set_attrib(GLint attrib)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer_);
    glVertexAttribPointer(
        attrib, array_type<T>::SIZE, array_type<T>::TYPE,
        GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}

#endif
