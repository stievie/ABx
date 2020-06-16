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

#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include "Utils.h"
#include <stdarg.h>
#include "ConsoleColor.h"
#include <sa/Assert.h>
#include <sa/Compiler.h>
#include <memory>
#include <thread>

// All 24h rotate log
#define LOG_ROTATE_INTERVAL (1000 * 60 * 60 * 24)

namespace IO {

/// Logger class with stream interface
class Logger
{
private:
    static std::unique_ptr<Logger> loggerInstance;
    enum class Mode
    {
        Stream,
        File
    };
#if defined(SA_PLATFORM_WIN)
    HANDLE hConsole_{ 0 };
    short foregroundDefault_{ 0 };
    bool isConsole_{ false };
#endif
    std::ofstream fstream_;
    std::ostream& stream_;
    Mode mode_;
    int64_t logStart_;
    bool nextIsBegin_{ true };
    using EndlType = decltype(std::endl<char, std::char_traits<char>>);
public:
    static std::string logDir_;

    explicit Logger(std::ostream& stream);
    explicit Logger(const std::string& fileName);
    ~Logger();

    // Overload for std::endl only:
    Logger& operator << (EndlType endl);
    // Some special types
    Logger& operator << (bool value)
    {
        stream_ << (value ? "true" : "false");
        return *this;
    }
    Logger& operator << (const std::thread::id& value)
    {
        std::stringstream ss;
        ss << std::hex << value;
        stream_ << ss.str();
        return *this;
    }
    // Everything else
    template <typename T>
    Logger& operator << (const T& data);

    Logger& Error(const char* function = nullptr, unsigned line = 0);
    Logger& Info(const char* function = nullptr, unsigned line = 0);
    Logger& Warning(const char* function = nullptr, unsigned line = 0);
#if defined(PROFILING)
    Logger& Profile();
#endif
    Logger& Debug(const char* function = nullptr, unsigned line = 0);
    static int PrintF(const char *__restrict __format, ...);

    static void Close();
    static Logger& Instance();
};

template <typename T>
inline Logger& Logger::operator << (const T& data)
{
    if (nextIsBegin_)
    {
        //set time_point to current time
        std::chrono::time_point<std::chrono::system_clock> time_point;
        time_point = std::chrono::system_clock::now();
        std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
        struct tm* p;
        p = localtime(&ttp);
        char chr[50] = { 0 };
        strftime(chr, 50, "(%g-%m-%d %H:%M:%S)", p);

        stream_ << std::string(chr) << " " << data;
        nextIsBegin_ = false;
    }
    else
    {
        stream_ << data;
    }
    return *this;
}

}

#define LOG_PLAIN (IO::Logger::Instance())
#define LOG_INFO (IO::Logger::Instance().Info())
#define LOG_WARNING (IO::Logger::Instance().Warning(SA_FUNCTION))
#define LOG_ERROR (IO::Logger::Instance().Error(SA_FUNCTION, __LINE__))
#if defined(PROFILING)
#   define LOG_PROFILE (IO::Logger::Instance().Profile())
#elif !defined(LOG_PROFILE)
#   define LOG_PROFILE
#endif
#define LOG_DEBUG (IO::Logger::Instance().Debug(SA_FUNCTION, __LINE__))
