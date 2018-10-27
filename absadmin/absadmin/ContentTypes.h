#pragma once

class ContentTypes
{
public:
    ContentTypes() = default;
    ~ContentTypes() = default;

    std::unordered_map<std::string, std::string> map_;

    const std::string& Get(const std::string& ext) const;
};

