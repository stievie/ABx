#pragma once

#include <unordered_map>
#include <functional>

namespace Utils {

template <typename IndexType, typename ReturnType, typename... _CArgs>
class CallableTable
{
private:
    std::unordered_map<IndexType, std::function<ReturnType(_CArgs...)>> callables_;
public:
    void Add(IndexType index, std::function<ReturnType(_CArgs...)>&& f)
    {
        callables_[index] = std::move(f);
    }
    ReturnType Call(IndexType index, _CArgs&& ... _Args)
    {
        auto& c = callables_[index];
        return c(std::forward<_CArgs>(_Args)...);
    }
    bool Exists(IndexType index) const
    {
        return callables_.find(index) != callables_.end();
    }
};

}
