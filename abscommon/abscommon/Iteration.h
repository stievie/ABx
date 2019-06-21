#pragma once

enum class Iteration
{
    Continue,
    Break
};

template<typename T, typename Callback>
void ForEach(T container, Callback&& callback)
{
    for (const auto& i : container)
        if (callback(i) != Iteration::Continue)
            break;
}
