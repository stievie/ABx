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
    AB::Entities::ModelClass modelClass_{ AB::Entities::ModelClassUnknown };
    bool stackAble_{ false };
};

String FormatMoney(uint32_t amount);
