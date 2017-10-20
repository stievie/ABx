#include "stdafx.h"
#include "Bans.h"
#include "Utils.h"

Bans Bans::Instance;

bool Bans::AcceptConnection(uint32_t clientIP)
{
    return true;
}
