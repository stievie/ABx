#pragma once

#include "NetworkMessage.h"

/// File Format
///  =============== Header ===============
///   4 Byte: Header == REC\0
///   sizeof(int16_t) Byte: File version == REC_FILE_VERSION
///   sizeof(uint32_t) Byte: overall size of network messages
///   char[36]: Game UUID
///   sizeof(int64_t) Byte: Game start time
///  ================ Body ================
///   --------- Network message ----------
///   sizeof(int32_t) Byte: Size of message
///   size Byte: The message
///   ------------------------------------
///   ...

namespace Game {
class Game;
}

namespace IO {

static constexpr int16_t REC_FILE_VERSION = 1;

class GameWriteStream
{
private:
    std::fstream stream_;
    bool open_;
    uint32_t size_;
public:
    GameWriteStream() :
        open_(false),
        size_(0)
    { }
    ~GameWriteStream();

    bool Open(const std::string& dir, Game::Game* game);
    void Close();
    void Write(const Net::NetworkMessage& msg);
    bool IsOpen() const
    {
        return open_;
    }
};

class GameReadStream
{
private:
    std::fstream stream_;
    bool open_;
    int64_t startTime_;
    uint32_t read_;
    uint32_t size_;
    std::string gameUuid_;
public:
    GameReadStream() :
        open_(false),
        startTime_(0),
        read_(0),
        size_(0)
    { }
    ~GameReadStream();

    bool Open(const std::string& dir, const std::string& instance);
    void Close();
    bool Read(Net::NetworkMessage& msg);
    bool IsOpen() const
    {
        return open_;
    }
    const std::string& GetGameUuid() const
    {
        return gameUuid_;
    }
};

}
