#pragma once

#include <fstream>
#include <chrono>
#include <ctime>
#include "Utils.h"
#include <stdarg.h>
#include "ConsoleColor.h"

#if !defined(__AB_PRETTY_FUNCTION__)
#   if defined __GNUC__
#       define __AB_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#   elif defined _MSC_VER
#       define __AB_PRETTY_FUNCTION__ __FUNCTION__
#   endif
#endif
// All 24h rotate log
#define LOG_ROTATE_INTERVAL (1000 * 60 * 60 * 24)

namespace IO {

/// Logger class with stream interface
class Logger
{
private:
    enum Mode
    {
        ModeStream,
        ModeFile
    };
    static std::unique_ptr<Logger> instance_;
    std::ofstream fstream_;
    std::ostream& stream_;
    Mode mode_;
    int64_t logStart_;
    using endlType = decltype(std::endl<char, std::char_traits<char>>);
public:
    static std::string logDir_;
    bool nextIsBegin_;

    explicit Logger(std::ostream& stream = std::cout) :
        stream_(stream),
        mode_(ModeStream),
        logStart_(Utils::Tick()),
        nextIsBegin_(true)
    {}
    explicit Logger(const std::string& fileName) :
        fstream_(fileName),
        stream_(fstream_),
        mode_(ModeFile),
        logStart_(Utils::Tick()),
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

    // Overload for std::endl only:
    Logger& operator << (endlType endl)
    {
#if !defined(AB_WINDOWS)
        static Color::Modifier def(Color::FG_DEFAULT);
#endif
        nextIsBegin_ = true;
#if !defined(AB_WINDOWS)
        if (mode_ == ModeStream)
            stream_ << def;
#endif
        stream_ << endl;
        return *this;
    }

    template <typename T>
    Logger& operator << (const T& data)
    {
        if (nextIsBegin_)
        {
            if (Utils::TimePassed(logStart_) > LOG_ROTATE_INTERVAL)
                Logger::Rotate();
            //set time_point to current time
            std::chrono::time_point<std::chrono::system_clock> time_point;
            time_point = std::chrono::system_clock::now();
            std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
            struct tm* p;
            p = localtime(&ttp);
            char chr[50] = { 0 };
            strftime(chr, 50, "(%g-%m-%d %H:%M:%S)", p);

            stream_ << std::string(chr) << ": " << data;
            nextIsBegin_ = false;
        }
        else
        {
            stream_ << data;
        }
        return *this;
    }

    void AddError(const std::string& msg)
    {
        Error() << msg << std::endl;
    }
    void AddInfo(const std::string& msg)
    {
        Info() << msg << std::endl;
    }
    void AddWarning(const std::string& msg)
    {
        Warning() << msg << std::endl;
    }
#if defined(_PROFILING)
    void AddProfile(const std::string& msg)
    {
        Profile() << msg << std::endl;
    }
#endif
#if defined(_DEBUG)
    void AddDebug(const std::string& msg)
    {
        Debug() << msg << std::endl;
    }
#endif
    Logger& Error()
    {
#if !defined(AB_WINDOWS)
        static Color::Modifier red(Color::FG_RED);
#endif
        if (nextIsBegin_)
        {
#if !defined(AB_WINDOWS)
            if (mode_ == ModeStream)
                stream_ << red;
#endif
            (*this) << "[ERROR] ";
        }
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
#if !defined(AB_WINDOWS)
        static Color::Modifier yellow(Color::FG_YELLOW);
#endif
        if (nextIsBegin_)
        {
#if !defined(AB_WINDOWS)
            if (mode_ == ModeStream)
                stream_ << yellow;
#endif
            (*this) << "[Warning] ";
        }
        return *this;
    }
#if defined(_PROFILING)
    Logger& Profile()
    {
#if !defined(AB_WINDOWS)
        static Color::Modifier blue(Color::FG_BLUE);
#endif
        if (nextIsBegin_)
        {
#if !defined(AB_WINDOWS)
            if (mode_ == ModeStream)
                stream_ << blue;
#endif
            (*this) << "[Profile] ";            
        }
        return *this;
    }
#endif
#if defined(_DEBUG)
    Logger& Debug()
    {
#if !defined(AB_WINDOWS)
        static Color::Modifier grey(Color::FG_LIGHTGREY);
#endif
        if (nextIsBegin_)
        {
#if !defined(AB_WINDOWS)
            if (mode_ == ModeStream)
                stream_ << grey;
#endif
            (*this) << "[Debug] ";
        }
        return *this;
    }
#endif
    static int PrintF(const char *__restrict __format, ...);

    static void Close()
    {
        Logger::instance_.reset();
    }
    static void Rotate()
    {
        if (!Logger::logDir_.empty())
        {
            Close();
            Logger::Instance();
        }
    }
    static Logger& Instance();
};

}

#define LOG_INFO (IO::Logger::Instance().Info())
#define LOG_WARNING (IO::Logger::Instance().Warning() << __AB_PRETTY_FUNCTION__ << "(): ")
#define LOG_ERROR (IO::Logger::Instance().Error() << __AB_PRETTY_FUNCTION__ << "(): ")
#if defined(_PROFILING)
#   define LOG_PROFILE (IO::Logger::Instance().Profile())
#endif
#if defined(_DEBUG)
#   define LOG_DEBUG (IO::Logger::Instance().Debug() << __AB_PRETTY_FUNCTION__ << "(): ")
#endif
