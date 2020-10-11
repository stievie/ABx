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


#include "GameStream.h"
#include "Game.h"
#include <abscommon/FileUtils.h>

namespace IO {

static constexpr int16_t REC_FILE_VERSION = 1;

GameWriteStream::~GameWriteStream()
{
    Close();
}

bool GameWriteStream::Open(const std::string& dir, Game::Game* game)
{
    filename_ = Utils::AddSlash(dir) + game->instanceData_.uuid + ".rec";
    stream_.open(filename_, std::ios::binary | std::fstream::out);
    open_ = stream_.is_open();
    if (open_)
    {
        // Write header
        stream_.write((char*)"REC\0", 4);
        stream_.write((char*)&REC_FILE_VERSION, sizeof(REC_FILE_VERSION));
        // Placeholder for size
        static const size_t PLACEHOLDER = 0;
        stream_.write((char*)&PLACEHOLDER, sizeof(size_t));

        stream_.write((char*)game->data_.uuid.c_str(), 36);
        stream_.write((char*)&game->startTime_, sizeof(decltype(game->startTime_)));
    }
    else
    {
        LOG_ERROR << "Unable to open file for writing: " << filename_ << std::endl;
    }
    return open_;
}

void GameWriteStream::Close()
{
    if (open_)
    {
        // Write size
        stream_.seekp(4 + sizeof(REC_FILE_VERSION), std::ios::beg);
        stream_.write((char*)&size_, sizeof(size_t));

        stream_.close();
        open_ = false;
    }
}

void GameWriteStream::Write(const Net::NetworkMessage& msg)
{
    if (open_)
    {
        uint32_t size = static_cast<uint32_t>(msg.GetSize());
        size_ += size;
        stream_.write((char*)&size, sizeof(size));
        stream_.write((char*)msg.GetBuffer(), size);
    }
}

GameReadStream::~GameReadStream()
{
    Close();
}

bool GameReadStream::Open(const std::string& dir, const std::string& instance)
{
    std::string filename = Utils::AddSlash(dir) + instance + ".rec";
    stream_.open(filename, std::ios::binary | std::fstream::in);
    open_ = stream_.is_open();
    if (open_)
    {
        char header[4] = { 0 };
        stream_.read((char*)&header, 4);
        if (header[0] != 'R' || header[1] != 'E' || header[2] != 'C' || header[3] != '\0')
        {
            LOG_ERROR << "Wrong file header" << std::endl;
            stream_.close();
            open_ = false;
            return false;
        }
        int16_t version;
        stream_.read((char*)&version, sizeof(version));
        if (version != REC_FILE_VERSION)
        {
            LOG_ERROR << "Wrong file version, got " << version << ", expected " << REC_FILE_VERSION << std::endl;
            stream_.close();
            open_ = false;
            return false;
        }
        stream_.read((char*)&size_, sizeof(size_t));

        gameUuid_.resize(36);
        stream_.read((char*)gameUuid_.data(), 36);
        stream_.read((char*)&startTime_, sizeof(startTime_));
    }
    else
        LOG_ERROR << "Unable to open file for reading: " << filename << std::endl;

    return open_;
}

void GameReadStream::Close()
{
    if (open_)
    {
        stream_.close();
        open_ = false;
    }
}

bool GameReadStream::Read(Net::NetworkMessage& msg)
{
    if (stream_.eof())
        return false;

    uint32_t size;
    stream_.read((char*)&size, sizeof(uint32_t));
    if (read_ + size > size_)
        return false;

    auto* buff = msg.GetBuffer();
    stream_.read((char*)buff, size);
    size_ += size;
    return true;
}

}
