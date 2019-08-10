#pragma once

#include <map>

namespace IO {
class PropReadStream;
class PropWriteStream;
}

namespace Utils {

enum VariantType
{
    VAR_NONE = 0,
    VAR_INT,
    VAR_INT64,
    VAR_BOOL,
    VAR_FLOAT,
    VAR_STRING,
    VAR_VOIDPTR
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
        type_(VAR_NONE),
        value_({ 0 }),
        stringValue_("")
    { }
    Variant(const Variant& variant) :
        type_(variant.type_),
        value_(variant.value_),
        stringValue_(variant.stringValue_)
    { }

    Variant(int value) : type_(VAR_INT) { value_.intValue = value; }
    Variant(unsigned value) : type_(VAR_INT) { value_.intValue = static_cast<int>(value); }
    Variant(long long value) : type_(VAR_INT64) { value_.int64Value = value; }
    Variant(unsigned long long value) : type_(VAR_INT64) { value_.int64Value = static_cast<long long>(value); }
    Variant(bool value) : type_(VAR_BOOL) { value_.boolValue = value; }
    Variant(float value) : type_(VAR_FLOAT) { value_.floatValue = value; }
    Variant(const std::string& value) : type_(VAR_STRING), value_{}, stringValue_(value) {}
    Variant(const char* value) : type_(VAR_STRING), value_{}, stringValue_(value) {}

    ~Variant() {}

    VariantType GetType() const { return type_; }
    void SetType(VariantType value) { type_ = value; }
    /// Check if it has a value
    bool IsEmpty() const
    {
        return type_ != VAR_NONE;
    }

    /// Assigning from other
    Variant& operator =(const Variant& other);
    Variant& operator =(int other)
    {
        SetType(VAR_INT);
        value_.intValue = other;
        return *this;
    }
    Variant& operator =(unsigned other)
    {
        SetType(VAR_INT);
        value_.intValue = static_cast<int>(other);
        return *this;
    }
    Variant& operator =(bool other)
    {
        SetType(VAR_BOOL);
        value_.boolValue = other;
        return *this;
    }
    Variant& operator =(float other)
    {
        SetType(VAR_FLOAT);
        value_.floatValue = other;
        return *this;
    }
    Variant& operator =(long long other)
    {
        SetType(VAR_INT64);
        value_.int64Value = other;
        return *this;
    }
    Variant& operator =(unsigned long long other)
    {
        SetType(VAR_INT64);
        value_.int64Value = static_cast<long long>(other);
        return *this;
    }
    Variant& operator =(const std::string& other)
    {
        SetType(VAR_STRING);
        stringValue_ = other;
        return *this;
    }
    Variant& operator =(const char* other)
    {
        SetType(VAR_STRING);
        stringValue_ = other;
        return *this;
    }
    Variant& operator =(void* other)
    {
        SetType(VAR_VOIDPTR);
        value_.ptrValue = other;
        return *this;
    }

    /// Test for equality with another variant.
    bool operator ==(const Variant& other) const;
    /// Test for equality. To return true, both the type and value must match.
    bool operator ==(int other) const { return type_ == VAR_INT ? value_.intValue == other : false; }
    bool operator ==(unsigned other) const { return type_ == VAR_INT ? value_.intValue == static_cast<int>(other) : false; }
    bool operator ==(bool other) const { return type_ == VAR_BOOL ? value_.boolValue == other : false; }
    bool operator ==(float other) const { return type_ == VAR_FLOAT ? value_.floatValue == other : false; }
    bool operator ==(long long other) const { return type_ == VAR_INT64 ? value_.int64Value == other : false; }
    bool operator ==(unsigned long long other) const { return type_ == VAR_INT64 ? value_.int64Value == static_cast<long long>(other) : false; }
    bool operator ==(const std::string& other) const { return type_ == VAR_STRING ? (stringValue_.compare(other) == 0) : false; }
    bool operator ==(const char* other) const { return type_ == VAR_STRING ? stringValue_.compare(other) == 0 : false; }
    bool operator ==(void* other) const { return type_ == VAR_VOIDPTR ? value_.ptrValue == other : false; }
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
        return (type_ == VAR_INT) ? value_.intValue : 0;
    }
    bool GetBool() const
    {
        return (type_ == VAR_BOOL) ? value_.boolValue : false;
    }
    float GetFloat() const
    {
        return (type_ == VAR_FLOAT) ? value_.floatValue : 0.0f;
    }
    long long GetInt64() const
    {
        return (type_ == VAR_INT64) ? value_.int64Value : 0;
    }
    const std::string& GetString() const
    {
        static const std::string STRING_EMPTY = "";
        return (type_ == VAR_STRING) ? *reinterpret_cast<const std::string*>(&stringValue_) : STRING_EMPTY;
    }
    void* GetPtr() const
    {
        return (type_ == VAR_VOIDPTR) ? value_.ptrValue : nullptr;
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

typedef std::map<size_t, Variant> VariantMap;
/// Empty variant map
const VariantMap VariantMapEmpty;

bool VariantMapRead(VariantMap& vMap, IO::PropReadStream& stream);
void VariantMapWrite(const VariantMap& vMap, IO::PropWriteStream& stream);

}
