#pragma once

#include <stdint.h>

namespace Utils {

class RefCounted
{
protected:
    uint32_t refCount_;
public:
    RefCounted(const RefCounted&) = delete;
    RefCounted& operator=(const RefCounted&) = delete;
    RefCounted() :
        refCount_(0)
    {}
    ~RefCounted() {}
    int32_t AddRef() { return ++refCount_; }
    int32_t ReleaseRef() { return --refCount_; }
    int32_t Refs() const { return refCount_; }
};

}
