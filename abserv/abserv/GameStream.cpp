#include "stdafx.h"
#include "GameStream.h"
#include "FileUtils.h"
#include "Game.h"
#include "Logger.h"

namespace IO {

GameWriteStream::~GameWriteStream()
{
    Close();
}

bool GameWriteStream::Open(const std::string& dir, Game::Game* game)
{
    std::string filename = Utils::AddSlash(dir) + game->instanceData_.uuid + ".rec";
    stream_.open(filename, std::ios::binary | std::fstream::out);
    open_ = stream_.is_open();
    if (open_)
    {
        // Write header
        stream_.write((char*)"REC\0", 4);
        stream_.write((char*)&REC_FILE_VERSION, sizeof(REC_FILE_VERSION));
        // Placeholder for size
        static const uint32_t PLACEHOLDER = 0;
        stream_.write((char*)&PLACEHOLDER, sizeof(uint32_t));
        stream_.write((char*)&game->startTime_, sizeof(decltype(game->startTime_)));
    }
    else
    {
        LOG_ERROR << "Unable to open file for writing: " << filename << std::endl;
    }
    return open_;
}

void GameWriteStream::Close()
{
    if (open_)
    {
        // Write size
        stream_.seekp(4 + sizeof(REC_FILE_VERSION), std::ios::beg);
        stream_.write((char*)&size_, sizeof(uint32_t));

        stream_.close();
        open_ = false;
    }
}

void GameWriteStream::Write(const Net::NetworkMessage& msg)
{
    if (open_)
    {
        auto size = msg.GetSize();
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
        if (header[0] != 'R' || header[1] != 'E' || header[2] != 'C' || header[4] != '\0')
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
        stream_.read((char*)&size_, sizeof(size_));
        stream_.read((char*)&startTime_, sizeof(decltype(startTime_)));
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

    int32_t size;
    stream_.read((char*)&size, sizeof(size));
    if (read_ + size > size_)
        return false;

    auto buff = msg.GetBuffer();
    stream_.read((char*)buff, size);
    size_ += size;
    return true;
}

}
