#pragma once

#include <AB/Entities/Item.h>

class Item : public Object
{
    URHO3D_OBJECT(Item, Object);
public:
    Item(Context* context);
    ~Item();

    template <class T>
    T* GetModelResource()
    {
        if (modelFile_.Empty())
            return nullptr;
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        return cache->GetResource<T>(modelFile_);
    }
    template <class T>
    T* GetIconResource()
    {
        if (iconFile_.Empty())
            return nullptr;
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        return cache->GetResource<T>(iconFile_);
    }

    String uuid_;
    uint32_t index_;
    String name_;
    String modelFile_;
    String iconFile_;
    AB::Entities::ItemType type_;
    bool stackAble_;
};

