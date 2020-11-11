/**
 * Copyright 2017-2020 Stefan Ascher
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

#include "OctreeQuery.h"
#include "GameObject.h"

namespace Math {

OctreeQuery::~OctreeQuery() = default;

bool OctreeQuery::Matches(const Game::GameObject* object) const
{
    if (!matcher_)
        return true;
    return matcher_->Matches(object);
}

Intersection PointOctreeQuery::TestOctant(const BoundingBox& box, bool inside)
{
    if (inside)
        return Intersection::Inside;
    else
        return box.IsInside(point_);
}

void PointOctreeQuery::TestObjects(Game::GameObject** start, Game::GameObject** end, bool inside)
{
    while (start != end)
    {
        Game::GameObject* object = *start++;

        if (object == ignore_ || !Matches(object))
            continue;
        if (inside || object->GetWorldBoundingBox().IsInside(point_) != Intersection::Outside)
            result_.push_back(object);
    }
}

Intersection SphereOctreeQuery::TestOctant(const BoundingBox& box, bool inside)
{
    if (inside)
        return Intersection::Inside;
    else
        return sphere_.IsInside(box);
}

void SphereOctreeQuery::TestObjects(Game::GameObject** start, Game::GameObject** end, bool inside)
{
    while (start != end)
    {
        Game::GameObject* object = *start++;

        if (object == ignore_ || !Matches(object))
            continue;
        if (inside || sphere_.IsInsideFast(object->GetWorldBoundingBox()) != Intersection::Outside)
            result_.push_back(object);
    }
}

Intersection BoxOctreeQuery::TestOctant(const BoundingBox& box, bool inside)
{
    if (inside)
        return Intersection::Inside;
    else
        return box_.IsInside(box);
}

void BoxOctreeQuery::TestObjects(Game::GameObject** start, Game::GameObject** end, bool inside)
{
    while (start != end)
    {
        Game::GameObject* object = *start++;
        if (object == ignore_ || !Matches(object))
            continue;
        if (inside || box_.IsInsideFast(object->GetWorldBoundingBox()) != Intersection::Outside)
            result_.push_back(object);
    }
}

}
