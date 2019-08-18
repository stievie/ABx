#include "stdafx.h"
#include "UuidUtils.h"
#include <uuid.h>

namespace Utils {
namespace Uuid {

bool IsEmpty(const std::string& uuid)
{
    return uuid.empty() || uuids::uuid(uuid).nil();
}

bool IsEqual(const std::string& u1, const std::string& u2)
{
    return (uuids::uuid(u1) == uuids::uuid(u2)) && !IsEmpty(u1);
}

std::string New()
{
    static uuids::uuid_system_generator generator = uuids::uuid_system_generator{};
    const uuids::uuid guid = generator();
    return guid.to_string();
}

}
}
