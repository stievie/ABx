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

#include "Logger.h"
#include "StringUtils.h"
#include <memory>

namespace IO {

std::string Logger::logDir_ = "";
std::unique_ptr<Logger> Logger::loggerInstance = nullptr;

Logger& Logger::Instance()
{
    if (loggerInstance)
    {
        if (Utils::TimeElapsed(loggerInstance->logStart_) > LOG_ROTATE_INTERVAL)
        {
            if (!Logger::logDir_.empty())
                loggerInstance.reset();
        }
    }

    if (!loggerInstance)
    {
        if (!logDir_.empty())
        {
            std::chrono::time_point<std::chrono::system_clock> time_point;
            time_point = std::chrono::system_clock::now();
            std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
            std::tm p = sa::time::localtime(ttp);
            std::string fn = sa::time::put_time(&p, "%g-%m-%d-%H-%M-%S");

            std::string logFile = Utils::ConcatPath(logDir_, fn, ".log");
            loggerInstance = std::make_unique<Logger>(logFile);
        }
        else
            loggerInstance = std::make_unique<Logger>(std::cout);
    }
    ASSERT(loggerInstance);
    return *loggerInstance;
}

void Logger::Close()
{
    loggerInstance.reset();
}

Logger::Logger(std::ostream& stream) :
    stream_(stream),
    mode_(Mode::Stream),
    logStart_(Utils::Tick())
{
#if defined(SA_PLATFORM_WIN)
    // Get default console font color on Windows
    hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO Info;
    GetConsoleScreenBufferInfo(hConsole_, &Info);
    foregroundDefault_ = Info.wAttributes;
    if (&stream_ == &std::cout)
    {
        isConsole_ = true;
        SetConsoleOutputCP(CP_UTF8);
    }
#endif
}

Logger::Logger(const std::string& fileName) :
    fstream_(fileName),
    stream_(fstream_),
    mode_(Mode::File),
    logStart_(Utils::Tick())
{}

Logger::~Logger()
{
    if (mode_ == Mode::File)
    {
        fstream_.flush();
        fstream_.close();
    }
}

Logger& Logger::operator << (EndlType endl)
{
#if !defined(SA_PLATFORM_WIN)
    static Color::Modifier def(Color::FG_DEFAULT);
#endif
    nextIsBegin_ = true;
    if (mode_ == Mode::Stream)
    {
#if !defined(SA_PLATFORM_WIN)
        stream_ << def;
#else
        SetConsoleTextAttribute(hConsole_, foregroundDefault_);
#endif
    }
    stream_ << endl;
#if defined(SA_PLATFORM_WIN)
    if (isConsole_)
        stream_ << std::flush;
#endif
    return *this;
}

Logger& Logger::Error(const char* function, unsigned line)
{
    static Color::Modifier red(Color::FG_LIGHTRED);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(SA_PLATFORM_WIN)
            stream_ << red;
#else
            SetConsoleTextAttribute(hConsole_, red.code_);
#endif
        }
        (*this) << "[ERROR " << std::this_thread::get_id() << "] ";
        if (function)
        {
            (*this) << function;
            if (line != 0)
                (*this) << ":" << line;
            (*this) << " ";
        }
    }
    return *this;
}

Logger& Logger::Info(const char* function, unsigned line)
{
    if (nextIsBegin_)
    {
        (*this) << "[Info " << std::this_thread::get_id() << "] ";
        if (function)
        {
            (*this) << function;
            if (line != 0)
                (*this) << ":" << line;
            (*this) << " ";
        }
    }
    return *this;
}

Logger& Logger::Warning(const char* function, unsigned line)
{
    static Color::Modifier yellow(Color::FG_LIGHTYELLOW);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(SA_PLATFORM_WIN)
            stream_ << yellow;
#else
            SetConsoleTextAttribute(hConsole_, yellow.code_);
#endif
        }
        (*this) << "[Warning " << std::this_thread::get_id() << "] ";
        if (function)
        {
            (*this) << function;
            if (line != 0)
                (*this) << ":" << line;
            (*this) << " ";
        }
    }
    return *this;
}

#if defined(PROFILING)
Logger& Logger::Profile()
{
    static Color::Modifier green(Color::FG_LIGHTGREEN);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(SA_PLATFORM_WIN)
            stream_ << green;
#else
            SetConsoleTextAttribute(hConsole_, green.code_);
#endif
        }
        (*this) << "[Profile " << std::this_thread::get_id() << "] ";
    }
    return *this;
}
#endif

Logger& Logger::Debug(const char* function, unsigned line)
{
    static Color::Modifier grey(Color::FG_LIGHTGREY);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(SA_PLATFORM_WIN)
            stream_ << grey;
#else
            SetConsoleTextAttribute(hConsole_, grey.code_);
#endif
        }
        (*this) << "[Debug " << std::this_thread::get_id() << "] ";
        if (function)
        {
            (*this) << function;
            if (line != 0)
                (*this) << ":" << line;
            (*this) << " ";
        }
    }
    return *this;
}

int Logger::PrintF(const char *__restrict __format, ...)
{
    int ret;

    /* Declare a va_list type variable */
    va_list myargs;

    /* Initialise the va_list variable with the ... after fmt */

    va_start(myargs, __format);

    char buff[1024] = { 0 };
    /* Forward the '...' to vprintf */
    ret = vsprintf(buff, __format, myargs);

    std::string msg(buff, static_cast<size_t>(ret));
    Instance() << msg;
    if (msg.back() == '\n')
        Instance().nextIsBegin_ = true;

    /* Clean up the va_list */
    va_end(myargs);

    return ret;
}

}

