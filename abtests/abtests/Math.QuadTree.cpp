#include <catch.hpp>

#include <absmath/QuadTree.h>
#include <vector>
#include <memory>

TEST_CASE("QuadTree Add")
{
    Math::QuadTreeBounds bounds(0, 0, 10, 10);
    Math::QuadTree tree(bounds);
    std::vector<std::unique_ptr<Math::QuadTreeObject>> objects;

    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            0,
            1,
            2,
            3
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            1,
            4,
            2,
            5
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            2,
            5,
            3,
            6
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            3,
            5,
            4,
            6
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            3,
            6,
            4,
            7
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }

    REQUIRE(tree.GetObjectCount() == 5);
}

TEST_CASE("QuadTree Query")
{
    Math::QuadTreeBounds bounds(0, 0, 10, 10);
    Math::QuadTree tree(bounds);
    std::vector<std::unique_ptr<Math::QuadTreeObject>> objects;

    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            0,
            1,
            2,
            3
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            1,
            4,
            2,
            5
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            2,
            5,
            3,
            6
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            3,
            5,
            4,
            6
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }
    {
        std::unique_ptr<Math::QuadTreeObject> object = std::make_unique<Math::QuadTreeObject>(Math::QuadTreeBounds(
            3,
            6,
            4,
            7
        ));
        tree.Insert(object.get());
        objects.push_back(std::move(object));
    }

    ea::vector<Math::QuadTreeObject*> result;
    tree.Query(Math::QuadTreeBounds(
        1,
        4,
        2,
        5
    ), result);
    REQUIRE(result.size() == 1);
}
