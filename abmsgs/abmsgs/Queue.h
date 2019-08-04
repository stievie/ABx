#pragma once

class Queue
{
private:
    std::string mapUuid_;
    std::vector<std::string> parties_;
    unsigned partySize_{ 0 };
    unsigned partyCount_{ 0 };
public:
    explicit Queue(const std::string mapUuid) :
        mapUuid_(mapUuid)
    {}
    bool Load();
    void Add(const std::string& partyUuid);
    void Remove(const std::string& partyUuid);

    size_t Count() const { return parties_.size(); }
};

