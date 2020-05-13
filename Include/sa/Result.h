/**
 * Copyright 2020 Stefan Ascher
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

// Result, Error pair

#include <iostream>
#include <utility>
#include <variant>
#include <sa/Assert.h>
#include <sa/StrongType.h>

namespace sa {

namespace details {
template <typename T>
using Ok = StrongType<T, struct OkTag>;
template <typename E>
using Error = StrongType<E, struct ErrorTag>;
}

template <typename T, typename E>
class Result
{
public:
    using OkType = details::Ok<T>;
    using ErrorType = details::Error<E>;
private:
    std::variant<OkType, ErrorType> value_;
public:
    Result() noexcept
    { }
    Result(const OkType& value) noexcept :
        value_(value)
    { }
    Result(const ErrorType& value) noexcept :
        value_(value)
    { }
    Result(OkType&& value) noexcept :
        value_(std::move(value))
    { }
    Result(ErrorType&& value) noexcept :
        value_(std::move(value))
    { }

    friend std::ostream& operator<<(std::ostream& os, const Result& rhs)
    {
        if (rhs.IsOk())
            os << static_cast<T>(std::get<OkType>(rhs.value_));
        else if (rhs.IsError())
            os << static_cast<E>(std::get<ErrorType>(rhs.value_));
        return os;
    }

    Result operator=(const OkType& value)
    {
        value_ = value;
        return *this;
    }
    Result operator=(const ErrorType& value)
    {
        value_ = value;
        return *this;
    }
    Result operator=(OkType&& value)
    {
        value_ = std::move(value);
        return *this;
    }
    Result operator=(ErrorType&& value)
    {
        value_ = std::move(value);
        return *this;
    }

    bool IsOk() const
    {
        return std::holds_alternative<OkType>(value_);
    }
    bool IsError() const
    {
        return std::holds_alternative<ErrorType>(value_);
    }

    explicit operator T() const
    {
        if (IsOk())
            return static_cast<T>(std::get<OkType>(value_));
        ASSERT_FALSE();
    }
    explicit operator OkType() const
    {
        if (IsOk())
            return static_cast<T>(std::get<OkType>(value_));
        ASSERT_FALSE();
    }
    explicit operator E() const
    {
        if (IsError())
            return static_cast<E>(std::get<ErrorType>(value_));
        ASSERT_FALSE();
    }
    explicit operator ErrorType() const
    {
        if (IsError())
            return static_cast<E>(std::get<ErrorType>(value_));
        ASSERT_FALSE();
    }
    operator bool() const
    {
        return IsOk();
    }
    bool operator==(T rhs) const
    {
        return IsOk() && (static_cast<T>(std::get<OkType>(value_)) == static_cast<T>(rhs));
    }
    bool operator!=(T rhs) const
    {
        return !(this == rhs);
    }
    bool operator==(E rhs) const
    {
        return IsError() && (static_cast<E>(std::get<ErrorType>(value_)) == static_cast<E>(rhs));
    }
    bool operator!=(E rhs) const
    {
        return !(this == rhs);
    }

    bool operator==(const Result& rhs) const
    {
        if (IsOk())
        {
            if (rhs.IsOk())
                return static_cast<T>(std::get<OkType>(value_)) == static_cast<T>(std::get<OkType>(rhs.value_));
            return false;
        }
        if (IsError())
        {
            if (rhs.IsError())
                return static_cast<E>(std::get<ErrorType>(value_)) == static_cast<E>(std::get<ErrorType>(rhs.value_));
        }
        return false;
    }

};

}
