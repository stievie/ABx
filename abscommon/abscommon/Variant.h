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

#include <map>
#include <sa/PropStream.h>

namespace Utils {

enum class VariantType
{
    None = 0,
    Int,
    Int64,
    Boolean,
    Float,
    String,
    VoidPtr
};

union VariantValue
{
    int intValue;
    int64_t int64Value;
    bool boolValue;
    float floatValue;
    void* ptrValue;
};

class Variant
{
private:
    VariantType type_;
    VariantValue value_;
    std::string stringValue_;
public:
    Variant() :
        type_(VariantType::None),
        value_({ 0 }),
        stringValue_("")
    { }
    Variant(const Variant& variant) :
        type_(variant.type_),
        value_(variant.value_),
        stringValue_(variant.stringValue_)
    { }

    Variant(int value) : type_(VariantType::Int) { value_.intValue = value; }
    Variant(unsigned value) : type_(VariantType::Int) { value_.intValue = static_cast<int>(value); }
    Variant(long long value) : type_(VariantType::Int64) { value_.int64Value = value; }
    Variant(unsigned long long value) : type_(VariantType::Int64) { value_.int64Value = static_cast<long long>(value); }
    Variant(bool value) : type_(VariantType::Boolean) { value_.boolValue = value; }
    Variant(float value) : type_(VariantType::Float) { value_.floatValue = value; }
    Variant(const std::string& value) : type_(VariantType::String), value_{}, stringValue_(value) {}
    Variant(const char* value) : type_(VariantType::String), value_{}, stringValue_(value) {}

    VariantType GetType() const { return type_; }
    void SetType(VariantType value) { type_ = value; }
    /// Check if it has a value
    bool IsEmpty() const
    {
        return type_ != VariantType::None;
    }

    /// Assigning from other
    Variant& operator =(const Variant& other);
    Variant& operator =(int other)
    {
        SetType(VariantType::Int);
        value_.intValue = other;
        return *this;
    }
    Variant& operator =(unsigned other)
    {
        SetType(VariantType::Int);
        value_.intValue = static_cast<int>(other);
        return *this;
    }
    Variant& operator =(bool other)
    {
        SetType(VariantType::Boolean);
        value_.boolValue = other;
        return *this;
    }
    Variant& operator =(float other)
    {
        SetType(VariantType::Float);
        value_.floatValue = other;
        return *this;
    }
    Variant& operator =(long long other)
    {
        SetType(VariantType::Int64);
        value_.int64Value = other;
        return *this;
    }
    Variant& operator =(unsigned long long other)
    {
        SetType(VariantType::Int64);
        value_.int64Value = static_cast<long long>(other);
        return *this;
    }
    Variant& operator =(const std::string& other)
    {
        SetType(VariantType::String);
        stringValue_ = other;
        return *this;
    }
    Variant& operator =(const char* other)
    {
        SetType(VariantType::String);
        stringValue_ = other;
        return *this;
    }
    Variant& operator =(void* other)
    {
        SetType(VariantType::VoidPtr);
        value_.ptrValue = other;
        return *this;
    }

    /// Test for equality with another variant.
    bool operator ==(const Variant& other) const;
    /// Test for equality. To return true, both the type and value must match.
    bool operator ==(int other) const { return type_ == VariantType::Int ? value_.intValue == other : false; }
    bool operator ==(unsigned other) const { return type_ == VariantType::Int ? value_.intValue == static_cast<int>(other) : false; }
    bool operator ==(bool other) const { return type_ == VariantType::Boolean ? value_.boolValue == other : false; }
    bool operator ==(float other) const { return type_ == VariantType::Float ? (value_.floatValue + std::numeric_limits<float>::epsilon() >= other &&
        value_.floatValue - std::numeric_limits<float>::epsilon() <= other) : false; }
    bool operator ==(long long other) const { return type_ == VariantType::Int64 ? value_.int64Value == other : false; }
    bool operator ==(unsigned long long other) const { return type_ == VariantType::Int64 ? value_.int64Value == static_cast<long long>(other) : false; }
    bool operator ==(const std::string& other) const { return type_ == VariantType::String ? (stringValue_.compare(other) == 0) : false; }
    bool operator ==(const char* other) const { return type_ == VariantType::String ? stringValue_.compare(other) == 0 : false; }
    bool operator ==(void* other) const { return type_ == VariantType::VoidPtr ? value_.ptrValue == other : false; }
    /// Test for inequality.
    bool operator !=(const Variant& other) const { return !(*this == other); }
    bool operator !=(int other) const { return !(*this == other); }
    bool operator !=(unsigned int other) const { return !(*this == other); }
    bool operator !=(bool other) const { return !(*this == other); }
    bool operator !=(float other) const { return !(*this == other); }
    bool operator !=(long long other) const { return !(*this == other); }
    bool operator !=(unsigned long long other) const { return !(*this == other); }
    bool operator !=(const std::string& other) const { return !(*this == other); }
    bool operator !=(const char* other) const { return !(*this == other); }
    bool operator !=(void* other) const { return !(*this == other); }

    std::string ToString() const;
    int GetInt() const
    {
        return (type_ == VariantType::Int) ? value_.intValue : 0;
    }
    bool GetBool() const
    {
        return (type_ == VariantType::Boolean) ? value_.boolValue : false;
    }
    float GetFloat() const
    {
        return (type_ == VariantType::Float) ? value_.floatValue : 0.0f;
    }
    long long GetInt64() const
    {
        return (type_ == VariantType::Int64) ? value_.int64Value : 0;
    }
    const std::string& GetString() const
    {
        static const std::string STRING_EMPTY = "";
        return (type_ == VariantType::String) ? *reinterpret_cast<const std::string*>(&stringValue_) : STRING_EMPTY;
    }
    void* GetPtr() const
    {
        return (type_ == VariantType::VoidPtr) ? value_.ptrValue : nullptr;
    }

    operator bool() const
    {
        return GetBool();
    }
    operator int() const
    {
        return GetInt();
    }
    operator unsigned() const
    {
        return static_cast<unsigned>(GetInt());
    }
    operator long long() const
    {
        return GetInt64();
    }
    operator unsigned long long() const
    {
        return static_cast<unsigned long long>(GetInt64());
    }
    operator float() const
    {
        return GetFloat();
    }
    operator std::string() const
    {
        return GetString();
    }

    /// Empty variant.
    static const Variant Empty;
};

template<class _Stream>
inline _Stream& operator << (_Stream& os, const Variant& value)
{
    os << value.ToString();
    return os;
}

typedef std::map<size_t, Variant> VariantMap;
/// Empty variant map
const VariantMap VariantMapEmpty;

bool VariantMapRead(VariantMap& vMap, sa::PropReadStream& stream);
void VariantMapWrite(const VariantMap& vMap, sa::PropWriteStream& stream);

}
