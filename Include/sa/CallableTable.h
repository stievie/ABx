#pragma once

#include <unordered_map>
#include <functional>

namespace sa {

template <typename IndexType, typename ReturnType, typename... _CArgs>
class CallableTable
{
public:
    using FunctionType = std::function<ReturnType(_CArgs...)>;
private:
    std::unordered_map<IndexType, FunctionType> callables_;
public:
    void Add(IndexType index, FunctionType&& f)
    {
        callables_[index] = std::move(f);
    }
    ReturnType Call(IndexType index, _CArgs&& ... _Args)
    {
        assert(Exists(index));
        auto& c = callables_[index];
        return c(std::forward<_CArgs>(_Args)...);
    }
    bool Exists(IndexType index) const
    {
        return callables_.find(index) != callables_.end();
    }
    FunctionType& operator [](IndexType index) {
        assert(Exists(index));
        return callables_[index];
    }
};

}
