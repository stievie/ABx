#include "stdafx.h"
#include "Variant.h"
#include <string>

#include "DebugNew.h"

namespace Utils {

const Variant Variant::Empty;

Variant& Variant::operator =(const Variant& other)
{
    SetType(other.GetType());

    switch (type_)
    {
    case VAR_STRING:
        stringValue_.assign(other.stringValue_);
        break;
    default:
        value_ = other.value_;
        break;
    }
    return *this;
}

bool Variant::operator ==(const Variant& other) const
{
    if (type_ != other.type_)
        return false;
    switch (type_)
    {
    case VAR_INT:
        return value_.intValue == other.value_.intValue;
    case VAR_BOOL:
        return value_.boolValue == other.value_.boolValue;
    case VAR_FLOAT:
        return value_.floatValue == other.value_.floatValue;
    case VAR_INT64:
        return value_.int64Value == other.value_.int64Value;
    case VAR_STRING:
        return stringValue_.compare(other.stringValue_) == 0;
    case VAR_VOIDPTR:
        return value_.ptrValue == other.value_.ptrValue;
    default:
        return true;
    }
}

std::string Variant::ToString() const
{
    switch (type_)
    {
    case VAR_INT:
        return std::to_string(value_.intValue);
    case VAR_BOOL:
        return std::to_string(value_.boolValue);
    case VAR_FLOAT:
        return std::to_string(value_.floatValue);
    case VAR_INT64:
        return std::to_string(value_.int64Value);
    case VAR_STRING:
        return stringValue_;
    case VAR_VOIDPTR:
        return std::to_string(0);
    default:
        return std::string();
    }
}

}
