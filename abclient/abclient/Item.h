#pragma once

#include <AB/Entities/Item.h>
#include <Urho3DAll.h>

class Item : public Object
{
    URHO3D_OBJECT(Item, Object)
public:
    Item(Context* context);
    ~Item() override;

    template <typename T>
    T* GetObjectResource()
    {
        if (objectFile_.Empty())
            return nullptr;
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        return cache->GetResource<T>(objectFile_);
    }
    template <typename T>
    T* GetIconResource()
    {
        if (iconFile_.Empty())
            return nullptr;
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        return cache->GetResource<T>(iconFile_);
    }

    String uuid_;
    uint32_t index_{ 0 };
    String name_;
    /// Prefab file
    String objectFile_;
    /// UI icon file, somewhere in /Textures
    String iconFile_;
    AB::Entities::ItemType type_{ AB::Entities::ItemTypeUnknown };
    AB::Entities::ModelClass modelClass_;
    bool stackAble_{ false };
};

