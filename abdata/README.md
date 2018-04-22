# abdata

Caching data server with Database backend.

## Characteristics

* Resistent to connection failures. It's even possible to restart the data server
while a client is connected to the server.
* Short Read, Update, Delete operations ~30µs. Create takes longer. This maakes it
possible that many clients read and write data very frequently (e.g. inside a
game loop) and still share the same data.
* Flushed data to the database server in a background thread (except for Create).
* Does not take care about database cobstraints.
* Does not use database generated values, like INSERT ID's. It uses application managed UUID's.
* Simple API, it has just 4 methods `Create()`, `Read()`, `Update()` and `Delete()`.
* Serialize any object over a network that is a descendant of `AB::Entities::Entity`.
* Type safety with C++ templates.
* Wow even works when its connected to MySQL while MySQL is updating!

## Entity

Example entity:

~~~cpp
#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_ACCOUNTS = "accounts";

enum AccountType : uint8_t
{
    AccountTypeUnknown = 0,
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

struct Account : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ACCOUNTS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(type);
        s.value1b(blocked);
        s.value8b(creation);
        s.text1b(name, Limits::MAX_ACCOUNT_NAME);
        s.text1b(password, Limits::MAX_ACCOUNT_PASS);
        s.text1b(email, Limits::MAX_ACCOUNT_EMAIL);
        s.value4b(charSlots);
    }

    uint8_t type = AccountTypeUnknown;
    bool blocked = false;
    uint64_t creation = 0;
    std::string name;
    std::string password;
    std::string email;
    uint32_t charSlots = 0;
};

}
}
~~~

## TODO

* How to check constraints?
* Use SSL?

## External Dependencies

* Boost

## Credits

* https://www.boost.org/
* https://think-async.com/Asio/AsioStandalone
* https://github.com/fraillt/bitsery
* https://github.com/mariusbancila/stduuid (modified to compile with VC++ 2015)
