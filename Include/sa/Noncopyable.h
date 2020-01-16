#pragma once

#define NON_COPYABLE(t)                \
private:                               \
    t(const t&) = delete;              \
    t& operator=(const t&) = delete;

#define NON_MOVEABLE                  \
private:                              \
    t(t&&) = delete;                  \
    t& operator=(t&&) = delete;
