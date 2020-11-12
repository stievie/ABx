/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <eastl.hpp>

namespace Math {

class QuadTreeBounds
{
public:
    QuadTreeBounds(float left, float top, float right, float bottom) :
        left_(left),
        top_(top),
        right_(right),
        bottom_(bottom),

        x_((left + right) * 0.5f),
        y_((top + bottom) * 0.5f),

        width_(bottom - top),
        height_(right - left)
    { }
    QuadTreeBounds(float left, float top, float right, float bottom,
        float x, float y) :
        left_(left),
        top_(top),
        right_(right),
        bottom_(bottom),

        x_(x),
        y_(y),

        width_(bottom - top),
        height_(right - left)
    { }
    QuadTreeBounds(float left, float top, float right, float bottom,
        float x, float y,
        float width, float height) :
        left_(left),
        top_(top),
        right_(right),
        bottom_(bottom),

        x_(x),
        y_(y),

        width_(width),
        height_(height)
    { }

    bool Intersects(const QuadTreeBounds& other) const
    {
        return (left_ <= other.left_ &&
            right_ >= other.right_ &&
            top_ <= other.top_&&
            bottom_ >= other.bottom_);
    }

    bool Contains(float x, float y) const
    {
        return ((x > left_) && (x < right_) &&
            (y > top_) && (y < bottom_));
    }
    bool IsDefined() const
    {
        return (width_ != 0 && height_ != 0) &&
            (left_ < right_) && (top_ < bottom_);
    }

    float left_;
    float top_;
    float right_;
    float bottom_;

    float x_;
    float y_;

    float width_;
    float height_;
};

class QuadTreeObject
{
public:
    QuadTreeObject(const QuadTreeBounds& bounds) :
        bounds_(bounds)
    { }
    QuadTreeObject(QuadTreeBounds&& bounds) :
        bounds_(std::move(bounds))
    { }
    QuadTreeBounds bounds_;
};

class QuadTree
{
private:
    static constexpr size_t NUM_CHILDREN = 4;
    static constexpr size_t CAPACITY = 4;

    QuadTreeBounds bounds_;
    QuadTreeBounds maxBounds_;

    ea::unique_ptr<QuadTree> children_[NUM_CHILDREN]{};
    ea::vector<QuadTreeObject*> objects_;
    QuadTree* root_{ nullptr };
    uint32_t level_{ 1 };
    bool haveChildren_{ false };

    void CalculateBounds();
    void Split();
    void Merge();
    void RemoveEmpty();
public:
    QuadTree(const QuadTreeBounds& bounds) :
        bounds_(bounds),
        maxBounds_(bounds)
    { }
    QuadTree(QuadTreeBounds&& bounds) :
        bounds_(std::move(bounds)),
        maxBounds_(bounds_)
    { }
    bool Insert(QuadTreeObject* object);
    bool Remove(QuadTreeObject* object);
    void Query(const QuadTreeBounds& bounds, ea::vector<QuadTreeObject*>& result);
    size_t GetObjectCount() const;
};

}
