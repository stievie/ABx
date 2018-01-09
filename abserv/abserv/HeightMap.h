#pragma once

namespace Math {

class HeightMap
{
public:
    HeightMap();
    ~HeightMap();

    std::vector<float> heightData_;
    float minHeight_;
    float maxHeight_;
};

}

