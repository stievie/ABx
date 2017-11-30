#pragma once

#include <stdint.h>

namespace AB {

enum ErrorCodes : uint8_t
{
    NoError = 0,
    Login_AccountNameOrPasswordWrong = 1,
};

}
