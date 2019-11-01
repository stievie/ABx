#include "stdafx.h"
#include "OctreeQuery.h"
#include "GameObject.h"

namespace Math {

OctreeQuery::~OctreeQuery() = default;

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

        if (object == ignore_)
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

        if (object == ignore_)
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
        if (object == ignore_)
            continue;
        if (inside || box_.IsInsideFast(object->GetWorldBoundingBox()) != Intersection::Outside)
            result_.push_back(object);
    }
}

}
