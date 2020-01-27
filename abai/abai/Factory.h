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

#include <memory>
#include <map>
#include "AiDefines.h"
#include <sa/Noncopyable.h>

namespace AI {

template <typename T>
class AbstractFactory
{
public:
    virtual ~AbstractFactory() = default;
    virtual std::shared_ptr<T> Create(const ArgumentsType& arguments) const = 0;
};

template <typename Key, typename T>
class FactoryRegistry
{
    NON_COPYABLE(FactoryRegistry)
protected:
    using FactoryMap = std::map<const Key, const AbstractFactory<T>*>;
    FactoryMap factories_;
public:
    FactoryRegistry() = default;

    bool RegisterFactory(const Key& key, const AbstractFactory<T>& factory)
    {
        const auto it = factories_.find(key);
        if (it != factories_.end())
            return false;

        factories_[key] = &factory;
        return true;
    }

    bool UnregisterFactory(const Key& key)
    {
        auto it = factories_.find(key);
        if (it == factories_.end())
            return false;
        factories_.erase(it);
        return true;
    }

    std::shared_ptr<T> Create(const Key& key, const ArgumentsType& arguments) const
    {
        const auto it = factories_.find(key);
        if (it == factories_.end())
            return std::shared_ptr<T>();

        const auto* factory = (*it).second;
        return factory->Create(arguments);
    }
};

}
