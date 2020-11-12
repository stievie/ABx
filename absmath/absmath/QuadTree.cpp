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

#include "QuadTree.h"

namespace Math {

void QuadTree::Split()
{
    if (haveChildren_)
        return;

    {
        children_[0] = std::make_unique<QuadTree>(QuadTreeBounds(bounds_.left_, bounds_.top_, bounds_.x_, bounds_.y_));
        children_[0]->level_ = level_ + 1;
        children_[0]->root_ = this;
    }
    {
        children_[1] = std::make_unique<QuadTree>(QuadTreeBounds(bounds_.x_, bounds_.top_, bounds_.right_, bounds_.y_));
        children_[1]->level_ = level_ + 1;
        children_[1]->root_ = this;
    }
    {
        children_[2] = std::make_unique<QuadTree>(QuadTreeBounds(bounds_.x_, bounds_.y_, bounds_.right_, bounds_.bottom_));
        children_[2]->level_ = level_ + 1;
        children_[2]->root_ = this;
    }
    {
        children_[3] = std::make_unique<QuadTree>(QuadTreeBounds(bounds_.left_, bounds_.y_, bounds_.x_, bounds_.bottom_));
        children_[3]->level_ = level_ + 1;
        children_[3]->root_ = this;
    }
    haveChildren_ = true;
}

void QuadTree::Merge()
{
    if (!haveChildren_)
        return;

    for (size_t i = 0; i < NUM_CHILDREN; ++i)
    {
        children_[i]->Merge();
        children_[i].reset();
    }
}

size_t QuadTree::GetObjectCount() const
{
    size_t result = objects_.size();
    if (haveChildren_)
    {
        for (size_t i = 0; i < NUM_CHILDREN; ++i)
            result += children_[i]->GetObjectCount();
    }
    return result;
}

void QuadTree::RemoveEmpty()
{
    if (!haveChildren_)
        return;

    for (size_t i = 0; i < NUM_CHILDREN; ++i)
        children_[i]->RemoveEmpty();
    if (GetObjectCount() == 0)
        Merge();

    haveChildren_ = false;
}

void QuadTree::CalculateBounds()
{
    maxBounds_ = bounds_;
    for (auto* object : objects_)
    {
        maxBounds_.left_ = std::min(maxBounds_.left_, object->bounds_.left_);
        maxBounds_.top_ = std::min(maxBounds_.top_, object->bounds_.top_);
        maxBounds_.right_ = std::max(maxBounds_.right_, object->bounds_.right_);
        maxBounds_.bottom_ = std::max(maxBounds_.bottom_, object->bounds_.bottom_);
    }

    if (haveChildren_)
    {
        for (size_t i = 0; i < NUM_CHILDREN; ++i)
        {
            maxBounds_.left_ = std::min(maxBounds_.left_, children_[i]->maxBounds_.left_);
            maxBounds_.top_ = std::min(maxBounds_.top_, children_[i]->maxBounds_.top_);
            maxBounds_.right_ = std::max(maxBounds_.right_, children_[i]->maxBounds_.right_);
            maxBounds_.bottom_ = std::max(maxBounds_.bottom_, children_[i]->maxBounds_.bottom_);
        }
    }

    maxBounds_.width_ = maxBounds_.bottom_ - maxBounds_.top_;
    maxBounds_.height_ = maxBounds_.right_ - maxBounds_.left_;
    maxBounds_.x_ = (maxBounds_.left_ + maxBounds_.right_) * 0.5f;
    maxBounds_.y_ = (maxBounds_.top_ + maxBounds_.bottom_) * 0.5f;

    if (root_)
        root_->CalculateBounds();
}

bool QuadTree::Insert(QuadTreeObject* object)
{
    if (!bounds_.Intersects(object->bounds_))
        return false;

    if (objects_.size() < CAPACITY)
    {
        objects_.push_back(object);
        CalculateBounds();
        return true;
    }

    if (!haveChildren_)
        Split();

    for (size_t i = 0; i < NUM_CHILDREN; ++i)
        if (children_[i]->Insert(object))
            return true;

    return false;
}

bool QuadTree::Remove(QuadTreeObject* object)
{
    if (!bounds_.Intersects(object->bounds_))
        return false;

    auto it = std::find_if(objects_.begin(), objects_.end(), [object](auto* current)
    {
        return current == object;
    });
    if (it != objects_.end())
    {
        objects_.erase(it);
        RemoveEmpty();
        CalculateBounds();
        return true;
    }

    if (!haveChildren_)
        return false;

    for (size_t i = 0; i < NUM_CHILDREN; ++i)
        if (children_[i]->Remove(object))
            return true;

    return false;
}

void QuadTree::Query(const QuadTreeBounds& bounds, ea::vector<QuadTreeObject*>& result)
{
    if (haveChildren_)
    {
        for (size_t i = 0; i < NUM_CHILDREN; ++i)
            children_[i]->Query(bounds, result);
    }
    for (auto* object : objects_)
    {
        if (object->bounds_.Intersects(bounds))
            result.push_back(object);
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
}

}