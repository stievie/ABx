/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <string>
#include <fstream>

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

namespace Net {
class NetworkMessage;
}

namespace Game {
class Game;
}

namespace IO {

class GameWriteStream
{
private:
    std::fstream stream_;
    bool open_;
    size_t size_;
    std::string filename_;
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
    std::string GetFilename() const { return filename_; }
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
