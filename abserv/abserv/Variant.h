#pragma once

#include <map>
#include <vector>

enum VariantType
{
    VAR_NONE = 0,
    VAR_INT,
    VAR_BOOL,
    VAR_FLOAT,
    VAR_TIME,
    VAR_STRING,
    VAR_VOIDPTR
};

struct VariantValue
{
    union
    {
        int intValue;
        bool boolValue;
        float floatValue;
        long long timeValue;               // milliseconds since the epoch of system_clock
        void* ptrValue;
    };
    VariantValue() :
        intValue(0),
        boolValue(false),
        floatValue(0.0f),
        timeValue(0),
        ptrValue(nullptr)
    { }
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
        stringValue_("")
    { }
    Variant(const Variant& variant) :
        type_(variant.type_),
        value_(variant.value_),
        stringValue_(variant.stringValue_)
    { }

    Variant(int value) : type_(VAR_INT) { value_.intValue = value; }
    Variant(bool value) : type_(VAR_BOOL) { value_.boolValue = value; }
    Variant(float value) : type_(VAR_FLOAT) { value_.floatValue = value; }
    Variant(long long value) : type_(VAR_TIME) { value_.timeValue = value; }
    Variant(const std::string& value) : type_(VAR_STRING), stringValue_(value) {}
    Variant(const char* value) : type_(VAR_STRING), stringValue_(value) {}

    ~Variant() {};

    VariantType GetType() const { return type_; }
    void SetType(VariantType value) { type_ = value; }

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
        value_.intValue = (int)other;
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
        SetType(VAR_TIME);
        value_.timeValue = other;
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
    bool operator ==(bool other) const { return type_ == VAR_BOOL ? value_.boolValue == other : false; }
    bool operator ==(float other) const { return type_ == VAR_FLOAT ? value_.floatValue == other : false; }
    bool operator ==(long long other) const { return type_ == VAR_TIME ? value_.timeValue == other : false; }
    bool operator ==(const std::string& other) const { return type_ == VAR_STRING ? (stringValue_.compare(other) == 0) : false; }
    bool operator ==(const char* other) const { return type_ == VAR_STRING ? stringValue_.compare(other) == 0 : false; }
    bool operator ==(void* other) const { return type_ == VAR_VOIDPTR ? value_.ptrValue == other : false; }
    /// Test for inequality.
    bool operator !=(const Variant& other) const { return !(*this == other); }
    bool operator !=(bool other) const { return !(*this == other); }
    bool operator !=(float other) const { return !(*this == other); }
    bool operator !=(long long other) const { return !(*this == other); }
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
    long long GetTime() const
    {
        return (type_ == VAR_TIME) ? value_.timeValue : 0;
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

    /// Empty variant.
    static const Variant Empty;
};

typedef std::map<size_t, Variant> VariantMap;
