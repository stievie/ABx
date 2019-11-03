#pragma once

#include <memory>
#include <map>

namespace AI {

template <typename T, typename Context>
class AbstractFactory
{
public:
    virtual ~AbstractFactory() = default;
    virtual std::shared_ptr<T> Create(const Context& ctx) const = 0;
};

template <typename Key, typename T, typename Context>
class FactoryRegistry
{
protected:
    using FactoryMap = std::map<const Key, const AbstractFactory<T, Context>*>;
    FactoryMap factores_;
public:
    FactoryRegistry() = default;
    FactoryRegistry(const FactoryRegistry&) = delete;
    FactoryRegistry& operator= (const FactoryRegistry&) = delete;

    bool RegisterFactory(const Key& key, const AbstractFactory<T, Context>& factory)
    {
        auto it = factores_.find(key);
        if (it != factores_.end())
            return false;

        factores_[key] = &factory;
    }

    bool UnregisterFactory(const Key& key)
    {
        auto it = factores_.find(key);
        if (it == factores_.end())
            return false;
        factores_.erase(it);
        return true;
    }

    std::shared_ptr<T> Create(const Key& key, const Context& ctx) const
    {
        const auto it = factores_.find(key);
        if (it == factores_.end())
            return std::shared_ptr<T>();

        const auto* factory = (*it).second;
        return factory->Create(ctx);
    }
};

}
