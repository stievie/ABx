#pragma once

namespace sa {

template <typename T>
class Transaction
{
private:
    T copy_;
    T& value_;
    bool committed_{ false };
public:
    Transaction(T& value) :
        copy_(value),
        value_(value)
    {}
    ~Transaction()
    {
        if (!committed_)
            value_ = copy_;
    }
    void Commit() { committed_ = true; }
};

}
