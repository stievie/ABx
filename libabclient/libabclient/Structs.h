#pragma once

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Quat
{
    float x;
    float y;
    float z;
    float w;
};

enum PlayerSex : uint8_t
{
    PlayerSexUnknown = 0,
    PlayerSexFemale,
    PlayerSexMale
};
