#pragma once

#include <stdint.h>

namespace IO {

enum class OpCodes : uint8_t
{
    None = 0,
    // Requests
    Create,
    Update,
    Read,
    Delete,
    // Invalidate a cache item. Flushes modified data. Next read will load it from the DB.
    Invalidate,
    Preload,
    Exists,
    // Clear all cache
    Clear,
    // Responses
    Status,
    Data
};

enum class ErrorCodes : uint8_t
{
    Ok,
    NoSuchKey,
    KeyTooBig,
    DataTooBig,
    OtherErrors,
    NotExists
};

}
