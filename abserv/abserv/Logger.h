#pragma once

#include <ostream>
#include <iostream>
#include <chrono>
#include <ctime>

#if defined __GNUC__
#define __AB_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#elif defined _MSC_VER
#define __AB_PRETTY_FUNCTION__ __FUNCTION__
#endif

class Logger
{
private:
    std::ostream& stream_;
    bool nextIsBegin_;
    Logger(std::ostream& stream = std::cout) :
        stream_(stream),
        nextIsBegin_(true)
    {}
    using endlType = decltype(std::endl<char, std::char_traits<char>>);
public:
    ~Logger() {}

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
    static Logger Instance;
};

#define LOG_INFO (Logger::Instance.Info())
#define LOG_WARNING (Logger::Instance.Warning() << __AB_PRETTY_FUNCTION__ << "(): ")
#define LOG_ERROR (Logger::Instance.Error() << __AB_PRETTY_FUNCTION__ << "(): ")
#ifdef _DEBUG
#define LOG_DEBUG (Logger::Instance.Debug() << __AB_PRETTY_FUNCTION__ << "(): ")
#endif
