#pragma once

#include <ostream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>

#if defined __GNUC__
#define __AB_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#elif defined _MSC_VER
#define __AB_PRETTY_FUNCTION__ __FUNCTION__
#endif

namespace IO {

class Logger
{
private:
    enum Mode
    {
        ModeStream,
        ModeFile
    };
    static std::unique_ptr<Logger> instance_;
    std::ostream& stream_;
    std::ofstream fstream_;
    bool nextIsBegin_;
    Mode mode_;
    using endlType = decltype(std::endl<char, std::char_traits<char>>);
public:
    static std::string logFile_;

    Logger(std::ostream& stream = std::cout) :
        stream_(stream),
        mode_(ModeStream),
        nextIsBegin_(true)
    {}
    Logger(const std::string& fileName) :
        fstream_(fileName),
        stream_(fstream_),
        mode_(ModeFile),
        nextIsBegin_(true)
    {}
    ~Logger()
    {
        if (mode_ == ModeFile)
        {
            fstream_.flush();
            fstream_.close();
        }
    }

    void Close()
    {
        Logger::instance_.reset();
    }

    //Overload for std::endl only:
    Logger& operator << (endlType endl)
    {

        nextIsBegin_ = true;
        stream_ << endl;
        return *this;
    }

    template <typename T>
    Logger& operator << (const T& data)
    {
        if (nextIsBegin_)
        {
            //set time_point to current time
            std::chrono::time_point<std::chrono::system_clock> time_point;
            time_point = std::chrono::system_clock::now();
            std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
            tm p;
            localtime_s(&p, &ttp);
            char chr[50];
            strftime(chr, 50, "(%g-%m-%d %H:%M:%S)", (const tm*)&p);

            stream_ << chr << ": " << data;
            nextIsBegin_ = false;
        }
        else
        {
            stream_ << data;
        }
        return *this;
    }
    Logger& Error()
    {
        if (nextIsBegin_)
            (*this) << "[ERROR] ";
        return *this;
    }
    Logger& Info()
    {
        if (nextIsBegin_)
            (*this) << "[Info] ";
        return *this;
    }
    Logger& Warning()
    {
        if (nextIsBegin_)
            (*this) << "[Warning] ";
        return *this;
    }
#ifdef _DEBUG
    Logger& Debug()
    {
        if (nextIsBegin_)
            (*this) << "[Debug] ";
        return *this;
    }
#endif
    static Logger& Instance()
    {
        if (!instance_)
        {
            if (!logFile_.empty())
            {
                instance_ = std::make_unique<Logger>(logFile_);
            }
            else
                instance_ = std::make_unique<Logger>();
        }
        return *instance_.get();
    }
};

}

#define LOG_INFO (IO::Logger::Instance().Info())
#define LOG_WARNING (IO::Logger::Instance().Warning() << __AB_PRETTY_FUNCTION__ << "(): ")
#define LOG_ERROR (IO::Logger::Instance().Error() << __AB_PRETTY_FUNCTION__ << "(): ")
#ifdef _DEBUG
#define LOG_DEBUG (IO::Logger::Instance().Debug() << __AB_PRETTY_FUNCTION__ << "(): ")
#endif
